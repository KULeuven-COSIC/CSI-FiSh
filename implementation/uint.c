
#include <stddef.h>

#include "params.h"
#include "uint.h"

/* assumes little-endian throughout. */

uint const uint_1 = {{1}};

void uint_set(uint *x, uint64_t y)
{
    x->c[0] = y;
    for (size_t i = 1; i < LIMBS; ++i)
        x->c[i] = 0;
}

bool uint_bit(uint const *x,  uint64_t k)
{
    return 1 & (x->c[k / 64] >> k % 64);
}


bool uint_add3(uint *x, uint const *y, uint const *z)
{
    bool c = 0;
    for (size_t i = 0; i < LIMBS; ++i) {
        uint64_t t;
        c = __builtin_add_overflow(y->c[i], c, &t);
        c |= __builtin_add_overflow(t, z->c[i], &x->c[i]);
    }
    return c;
}

bool uint_sub3(uint *x, uint const *y, uint const *z)
{
    bool b = 0;
    for (size_t i = 0; i < LIMBS; ++i) {
        uint64_t t;
        b = __builtin_sub_overflow(y->c[i], b, &t);
        b |= __builtin_sub_overflow(t, z->c[i], &x->c[i]);
    }
    return b;
}

void uint_mul3_64(uint *x, uint const *y, uint64_t z)
{
    uint64_t c = 0;
    for (size_t i = 0; i < LIMBS; ++i) {
        __uint128_t t = y->c[i] * (__uint128_t) z + c;
        c = t >> 64;
        x->c[i] = t;
    }
}

