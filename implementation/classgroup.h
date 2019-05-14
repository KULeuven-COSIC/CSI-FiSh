#ifndef CLASSGROUP_H
#define CLASSGROUP_H

#include "gmp.h"
#include "stdint.h"
#include <stdio.h>
#include <assert.h>
#include "reduce.h"
#include "parameters.h"
#include "params.h"
#include <openssl/rand.h>

extern mpz_t cn;

void init_classgroup();
void clear_classgroup();

void sample_mod_cn(mpz_t a);
void mod_cn_2_vec(mpz_t a, int8_t *vec);
void sample_from_classgroup(int8_t *vec);

void sample_mod_cn_with_seed(const unsigned char *seed, mpz_t a);
void sample_from_classgroup_with_seed(const unsigned char *seed, int8_t *vec);

void shrink_vec(int8_t *vec);

void test();


#endif