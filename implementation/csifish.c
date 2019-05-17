#include "csifish.h"

void print_uint(uint x){
	for(int i=0 ; i<LIMBS; i++){
		printf("%lu ", x.c[i] );
	}
	printf("\n");
}

void csifish_keygen(unsigned char *pk, unsigned char *sk){
	// pick random root seed
	RAND_bytes(sk, SEED_BYTES);

	#ifdef MERKLEIZE_PK
		// pick merkle tree key
		RAND_bytes(pk + SEED_BYTES, SEED_BYTES );
		memcpy(SK_MERKLE_KEY(sk),pk+SEED_BYTES,SEED_BYTES);
	#endif

	init_classgroup();

	// generate seeds
	unsigned char *seeds = malloc(SEED_BYTES*PKS);
	EXPAND(sk,SEED_BYTES,seeds,SEED_BYTES*PKS);

	// generate public key curves
	#ifdef MERKLEIZE_PK
		uint *curves = malloc(sizeof(uint[PKS]));
	#else
		uint* curves = (uint*) PK_CURVES(pk);
	#endif

	for (int i = 0; i < PKS; ++i)
	{
		private_key vec;
		sample_from_classgroup_with_seed(seeds + i*SEED_BYTES,vec.e);

		// compute E_o * vec
		public_key out;
		action(&out, &base, &vec);

		// convert to uint64_t's
		fp_dec(&curves[i], &out.A);
	}

	#ifdef MERKLEIZE_PK
		// build merkle tree on all the curves
		build_tree((unsigned char *) curves, sizeof(uint), PK_TREE_DEPTH, SK_TREE(sk),SK_MERKLE_KEY(sk));

		// copy root of tree to pk
		memcpy(pk,SK_TREE(sk), SEED_BYTES);
	#endif
	clear_classgroup();
	free(seeds);
	#ifdef MERKLEIZE_PK
		free(curves);
	#endif
}

void get_challenges(const unsigned char *hash, uint32_t *challenges_index, uint8_t *challenges_sign){
	unsigned char tmp_hash[SEED_BYTES];
	memcpy(tmp_hash,hash,SEED_BYTES);

	// slow hash function
	for(int i=0; i<HASHES; i++){
		HASH(tmp_hash,SEED_BYTES,tmp_hash);
	}

	// generate pseudorandomness
	EXPAND(tmp_hash,SEED_BYTES,(unsigned char *) challenges_index,sizeof(uint32_t)*ROUNDS);

	// set sign bit and zero out higher order bits
	for(int i=0; i<ROUNDS; i++){
		challenges_sign[i] = (challenges_index[i] >> PK_TREE_DEPTH) & 1;
		challenges_index[i] &= (((uint16_t) 1)<<PK_TREE_DEPTH)-1;
	}
}

void csifish_sign(const unsigned char *sk,const unsigned char *m, uint64_t mlen, unsigned char *sig, uint64_t *sig_len){
	init_classgroup();

	// hash the message
	unsigned char m_hash[HASH_BYTES];
	HASH(m,mlen,m_hash);

	// pick random seeds
	unsigned char seeds[SEED_BYTES*ROUNDS];
	RAND_bytes(seeds,SEED_BYTES*ROUNDS);

	// compute curves
	mpz_t r[ROUNDS];
	uint curves[ROUNDS] = {{{0}}};
	for(int k=0 ; k<ROUNDS; k++){
		private_key priv;

		// sample mod class number and convert to vector
		mpz_init(r[k]);
		sample_mod_cn_with_seed(seeds + k*SEED_BYTES,r[k]);
		mod_cn_2_vec(r[k],priv.e);

		// compute E_o * vec
		public_key out;
		action(&out, &base, &priv);

		// convert to uint64_t's
		fp_dec(&curves[k], &out.A);
	}

	#ifdef MERKLEIZE_PK
		// copy curves to signature
		memcpy(SIG_CURVES(sig),(unsigned char *) curves, sizeof(uint[ROUNDS]));
	#endif

	// hash curves
	unsigned char curve_hash[HASH_BYTES];
	HASH((unsigned char *) curves, sizeof(uint[ROUNDS]), curve_hash);

	// compute master hash
	unsigned char in_buf[2*HASH_BYTES], master_hash[HASH_BYTES];
	memcpy(in_buf,m_hash,HASH_BYTES);
	memcpy(in_buf + HASH_BYTES, curve_hash, HASH_BYTES);
	HASH(in_buf,2*HASH_BYTES, master_hash);

	#ifndef MERKLEIZE_PK
		// copy hash to signature
		memcpy(SIG_HASH(sig),master_hash,HASH_BYTES);
	#endif

	// get challenges
	uint32_t challenges_index[ROUNDS];
	uint8_t challenges_sign[ROUNDS];
	get_challenges(master_hash,challenges_index,challenges_sign);

	// generate seeds
	unsigned char *sk_seeds = malloc(SEED_BYTES*PKS);
	EXPAND(sk,SEED_BYTES,sk_seeds,SEED_BYTES*PKS);

	// generate secrets mod p
	unsigned char *indices = calloc(1,PKS);
	(void) indices;
	mpz_t s[ROUNDS];
	for(int i=0; i<ROUNDS; i++){
		indices[challenges_index[i]] = 1;
		mpz_init(s[i]);
		sample_mod_cn_with_seed(sk_seeds + challenges_index[i]*SEED_BYTES ,s[i]);
		if(challenges_sign[i]){
			mpz_mul_si(s[i],s[i],-1);
		}
		mpz_sub(r[i],s[i],r[i]);
		mpz_fdiv_r(r[i],r[i],cn);

		// silly trick to force export to have 33 bytes
		mpz_add(r[i],r[i],cn);

		mpz_export(SIG_RESPONSES(sig) + 33*i, NULL, 1, 1, 1, 0, r[i]);

		mpz_clear(s[i]);
		mpz_clear(r[i]);
	}


	#ifdef MERKLEIZE_PK
		// release the necessary nodes of merkle tree
		uint16_t nodes_released;
		release_nodes(SK_TREE(sk), SEED_BYTES, PK_TREE_DEPTH, indices, SIG_TREE_FILLING(sig) , &nodes_released);	
	
		// update signature length
		(*sig_len) = (SIG_TREE_FILLING(0) + nodes_released*SEED_BYTES);
	#else
		(*sig_len) = SIG_BYTES;
	#endif

	clear_classgroup();
	free(indices);
	free(sk_seeds);
}


#ifdef MERKLEIZE_PK

int csifish_verify(const unsigned char *pk, const unsigned char *m, uint64_t mlen, const unsigned char *sig, uint64_t sig_len){
	init_classgroup();

	// hash the message
	unsigned char m_hash[HASH_BYTES];
	HASH(m,mlen,m_hash);

	// hash curves
	uint* curves = (uint*) SIG_CURVES(sig);
	unsigned char curve_hash[HASH_BYTES];
	HASH((unsigned char *) curves, sizeof(uint[ROUNDS]), curve_hash);

	// compute master hash
	unsigned char in_buf[2*HASH_BYTES], master_hash[HASH_BYTES];
	memcpy(in_buf,m_hash,HASH_BYTES);
	memcpy(in_buf + HASH_BYTES, curve_hash, HASH_BYTES);
	HASH(in_buf,2*HASH_BYTES, master_hash);

	// get challenges
	uint32_t challenges_index[ROUNDS];
	uint8_t challenges_sign[ROUNDS];
	get_challenges(master_hash,challenges_index,challenges_sign);


	fp minus_one;
	fp_sub3(&minus_one, &fp_0, &fp_1);

	uint *pkcurves = malloc(sizeof(uint[PKS])); 
	unsigned char *indices = calloc(1,PKS);
	for(int i=0; i<ROUNDS; i++){
		// encode starting point
		public_key start,end;
		fp_enc(&(start.A), &curves[i]);

		// decode path
		mpz_t x;
		mpz_init(x);
		private_key path;
		mpz_import(x,33,1,1,1,0,SIG_RESPONSES(sig)+33*i);
		mpz_sub(x,x,cn);
		mod_cn_2_vec(x,path.e);
		mpz_clear(x);

		// perform action
		action(&end,&start,&path);

		if(challenges_sign[i]){
			fp_mul2(&end.A,&minus_one);
		}

		// decode endpoint
		uint pkcurve;
		fp_dec(&pkcurve,&end.A);	

		// put endpoint in pkcurves, or compare to earlier found curve
		if(indices[challenges_index[i]] == 0){
			pkcurves[challenges_index[i]] = pkcurve;
		}
		else if( memcmp(&pkcurves[challenges_index[i]],&pkcurve,sizeof(uint)) ){
			// curve not equal to earlier found curve! reject signature
			clear_classgroup();
			free(indices);
			free(pkcurves);
			return -1;
		}
		indices[challenges_index[i]] = 1;
	}

	clear_classgroup();

	// reconstruct root of merkle tree
	unsigned char root[SEED_BYTES];
	int nodes = ( sig_len - SIG_TREE_FILLING(0))/SEED_BYTES;
	hash_up(pkcurves, indices,SIG_TREE_FILLING(sig),nodes,root,pk+SEED_BYTES);

	free(indices);
	free(pkcurves);

	// compare root with pk
	if(memcmp(root,pk,SEED_BYTES)){
		return -1;
	}

	return 1;
}

#else

int csifish_verify(const unsigned char *pk, const unsigned char *m, uint64_t mlen, const unsigned char *sig, uint64_t sig_len){
	init_classgroup();
	(void)sig_len;

	// hash the message
	unsigned char m_hash[HASH_BYTES];
	HASH(m,mlen,m_hash);

	// get challenges
	uint32_t challenges_index[ROUNDS];
	uint8_t  challenges_sign[ROUNDS];
	get_challenges(SIG_HASH(sig),challenges_index,challenges_sign);

	fp minus_one;
	fp_sub3(&minus_one, &fp_0, &fp_1);

	uint  curves[ROUNDS];
	uint* pkcurves = (uint*) PK_CURVES(pk); 
	for(int i=0; i<ROUNDS; i++){
		// encode starting point
		public_key start,end;
		fp_enc(&(start.A), &pkcurves[challenges_index[i]]);

		if(challenges_sign[i]){
			fp_mul2(&start.A,&minus_one);
		}

		// decode path
		mpz_t x;
		mpz_init(x);
		private_key path;
		mpz_import(x,33,1,1,1,0,SIG_RESPONSES(sig)+33*i);
		mpz_sub(x,x,cn);
		mod_cn_2_vec(x,path.e);
		mpz_clear(x);

		// flip vector
		for(int j=0; j<NUM_PRIMES; j++){
			path.e[j] = -path.e[j];
		}

		// perform action
		action(&end,&start,&path);

		// decode endpoint
		fp_dec(&curves[i],&end.A);	
	}

	clear_classgroup();

	// hash curves
	unsigned char curve_hash[HASH_BYTES];
	HASH((unsigned char *) curves, sizeof(uint[ROUNDS]), curve_hash);

	// compute master hash
	unsigned char in_buf[2*HASH_BYTES], master_hash[HASH_BYTES];
	memcpy(in_buf,m_hash,HASH_BYTES);
	memcpy(in_buf + HASH_BYTES, curve_hash, HASH_BYTES);
	HASH(in_buf,2*HASH_BYTES, master_hash);

	// compare master_hash with signature_hash
	if(memcmp(master_hash,SIG_HASH(sig),HASH_BYTES)){
		return -1;
	}

	return 1;
}

#endif
