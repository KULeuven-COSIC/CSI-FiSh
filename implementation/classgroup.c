#include "classgroup.h"

#include "HKZbasis.c"
//#include "BKZ50.c"
//#include "BKZ40.c"

mpz_t cn, half_cn, twopow258, babai_Ainv_row[NUM_PRIMES];
mpf_t IP[NUM_PRIMES],B[NUM_PRIMES*NUM_PRIMES];

const char A[NUM_PRIMES*NUM_PRIMES];

// clear classgroup variables
void clear_classgroup(){
	mpz_clear(cn);
	mpz_clear(twopow258);

	for(int i=0; i<NUM_PRIMES; i++){
		mpz_clear(babai_Ainv_row[i]);
	}
}

void sample_mod_cn_with_seed(const unsigned char *seed, mpz_t a){	
	unsigned char in_buf[SEED_BYTES+1];
	memcpy(in_buf,seed,SEED_BYTES);
	in_buf[SEED_BYTES] = 0;
  	while(1){
  		// get random bytes
  		unsigned char randomness[33];
		EXPAND(in_buf,SEED_BYTES+1,randomness,33);
		in_buf[SEED_BYTES]++;

  		// import from randomness
  		mpz_import(a,33,1,1,0,0,randomness);

  		// reduce mod 2^258
  		mpz_tdiv_r(a, a, twopow258);

  		if( mpz_cmp(cn,a) > 0){
  			break;
  		}
  	}
}
 
void sample_mod_cn(mpz_t a){	
	// pick random seed
  	unsigned char seed[SEED_BYTES];
  	RAND_bytes(seed,SEED_BYTES);

  	// sample with seed
  	sample_mod_cn_with_seed(seed,a);
}

void inner_product(mpz_t *a, mpf_t *b, mpf_t out){
	mpf_init(out);

	for (int i = 0; i < NUM_PRIMES; ++i)
	{
		// convert from integer to float
		mpf_t tmp;
		mpf_init(tmp);
		mpf_set_z(tmp,a[i]);

		// multiply and add
		mpf_mul(tmp,tmp,b[i]);
		mpf_add(out,out,tmp);

		mpf_clear(tmp);
	}
}

void sub_multiple(mpz_t *target, const char *vector, mpz_t multiple){
	for (int i = 0; i < NUM_PRIMES; ++i)
	{
		mpz_t tmp;
		mpz_init(tmp);
		mpz_mul_si(tmp, multiple, vector[i]);

		mpz_sub(target[i],target[i],tmp);

		mpz_clear(tmp);
	}
}

// computes L1 norm of vector
int32_t L1( int8_t *vec ){
	int32_t sum = 0;
	for(int i=0; i< NUM_PRIMES ; i++){
		sum += abs(vec[i]);
	}
	return sum;
}

// convert an integer modulo class number to a short vector modulo the relation lattice
void mod_cn_2_vec(mpz_t a, int8_t *vec){
	
	// initialize target
	mpz_t target[NUM_PRIMES];
	for(int i=0; i<NUM_PRIMES; i++){
		mpz_init(target[i]);
	}
	mpz_set(target[0], a);

	// babai nearest plane
	for(int i=NUM_PRIMES-1 ; i>=0 ; i--){
		mpf_t ip1,floor;
		inner_product(target,B + i*74 ,ip1);
		mpf_div(ip1,ip1,IP[i]);

		mpf_init(floor);
		mpf_floor(floor,ip1);

		mpf_t remainder;
		mpf_init(remainder);
		mpf_sub(remainder,ip1,floor);

		if( mpf_cmp_d(remainder,0.5) > 0 ){
			mpf_add_ui(floor,floor,1);
		}

		mpz_t r;
		mpz_init(r);
		mpz_set_f(r,floor);

		sub_multiple(target, A + 74*i, r);

		mpz_clear(r);
		mpf_clear(ip1);
		mpf_clear(floor);
		mpf_clear(remainder);
	}

	for(int i=0; i<NUM_PRIMES ; i++ ){
		vec[i] = mpz_get_si(target[i]);
		mpz_clear(target[i]);
	}

	//int norm = L1(vec);

	// reduce with pool of small vectors
	reduce(vec,2,10000);

	//printf("norm before reduction %d \n"  , norm );
	//printf("norm after  reduction %d \n"  , L1(vec));
	//printf("improvement           %d \n\n", norm-L1(vec));
}

void sample_from_classgroup(int8_t *vec){
	// pick random seed
	unsigned char seed[SEED_BYTES];
  	RAND_bytes(seed,SEED_BYTES);

  	// sample with seed
  	sample_from_classgroup_with_seed(seed,vec);
}

void sample_from_classgroup_with_seed(const unsigned char *seed, int8_t *vec){
	mpz_t a;
	mpz_init(a);

	// get random number mod class_number
	sample_mod_cn_with_seed(seed,a);

	// convert to vector with babai rounding
	mod_cn_2_vec(a,vec);
	mpz_clear(a);
}

// initialize classgroup constants
void init_classgroup(){
	mpf_set_default_prec (1000);

	const char* cnstring = "254652442229484275177030186010639202161620514305486423592570860975597611726191";
	mpz_init_set_str(cn, cnstring, 10);

	const char* half_cnstring = "127326221114742137588515093005319601080810257152743211796285430487798805863095";
	mpz_init_set_str(half_cn, half_cnstring, 10);

	const char* string = "1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	mpz_init_set_str(twopow258, string, 2);

	for(int i=0; i<NUM_PRIMES; i++){
		mpf_init(IP[i]);
		mpf_set_str(IP[i],IPstrings[i] , 10);
	}

	for(int i=0; i<NUM_PRIMES*NUM_PRIMES; i++){
		mpf_init(B[i]);
		mpf_set_str(B[i],Bstrings[i], 10);
	}
}