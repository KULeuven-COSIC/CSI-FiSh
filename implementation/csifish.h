#ifndef CSIFISH_H
#define CSIFISH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csidh.h"
#include "merkletree.h"
#include "stdint.h"
#include "parameters.h"
#include "classgroup.h"
#include "fp.h"

#ifdef MERKLEIZE_PK

	#define PK_BYTES SEED_BYTES+SEED_BYTES

	#define SK_SEED(sk) (sk)
	#define SK_MERKLE_KEY(sk) (SK_SEED(sk) + SEED_BYTES)
	#define SK_TREE(sk) (SK_MERKLE_KEY(sk) + SEED_BYTES)
	#define SK_BYTES (2*SEED_BYTES + ((2*PKS-1)*SEED_BYTES))

	#define SIG_CURVES(sig) (sig) 
	#define SIG_RESPONSES(sig) (SIG_CURVES(sig) + sizeof(uint[ROUNDS]))
	#define SIG_TREE_FILLING(sig) (SIG_RESPONSES(sig) + 33*ROUNDS)
	#define SIG_BYTES (SIG_TREE_FILLING(0) + 10000)

#else
	
	#define PK_CURVES(pk) (pk)
	#define PK_BYTES sizeof(uint[PKS])

	#define SK_SEED(sk) (sk)
	#define SK_BYTES SEED_BYTES 

	#define SIG_HASH(sig) (sig)
	#define SIG_RESPONSES(sig) (SIG_HASH(sig) + HASH_BYTES)
	#define SIG_BYTES (SIG_RESPONSES(0) + 33*ROUNDS)

#endif

void csifish_keygen(unsigned char *pk, unsigned char *sk);
void csifish_sign(const unsigned char *sk,const unsigned char *m, uint64_t mlen, unsigned char *sig, uint64_t *sig_len);
int csifish_verify(const unsigned char *pk, const unsigned char *m, uint64_t mlen, const unsigned char *sig, uint64_t sig_len);

#endif