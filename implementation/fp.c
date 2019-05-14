
#include <stddef.h>
#include <string.h>

#include "params.h"
#include "uint.h"
#include "fp.h"
#include "rng.h"

void fp_set(fp *x, uint64_t y)
{
    uint_set((uint *) x, y);
    fp_enc(x, (uint *) x);
}

static void reduce_once(uint *x)
{
    uint t;
    if (!uint_sub3(&t, x, &p))
        *x = t;
}

void fp_add3(fp *x, fp const *y, fp const *z)
{
    uint_add3((uint *) x, (uint *) y, (uint *) z);
    reduce_once((uint *) x);
}

void fp_add2(fp *x, fp const *y)
{
    fp_add3(x, x, y);
}

void fp_sub3(fp *x, fp const *y, fp const *z)
{
    if (uint_sub3((uint *) x, (uint *) y, (uint *) z))
        uint_add3((uint *) x, (uint *) x, &p);
}

void fp_sub2(fp *x, fp const *y)
{
    fp_sub3(x, x, y);
}


/* Montgomery arithmetic */

void fp_enc(fp *x, uint const *y)
{
    fp_mul3(x, (fp *) y, &r_squared_mod_p);
}

void fp_dec(uint *x, fp const *y)
{
    fp_mul3((fp *) x, y, (fp *) &uint_1);
}

void fp_mul3(fp *x, fp const *y, fp const *z)
{
    uint64_t t[LIMBS + 1] = {0};
    for (size_t k = 0; k < LIMBS; ++k) {
#define r(i) t[(k + (i)) % (LIMBS + 1)]

        uint64_t m = inv_min_p_mod_r * (y->c[k] * z->c[0] + r(0));

        bool c = 0, o = 0;
        for (size_t i = 0; i < LIMBS; ++i) {
            __uint128_t u = (__uint128_t) m * p.c[i];
            o = __builtin_add_overflow(r(i), o, &r(i));
            o |= __builtin_add_overflow(r(i), (uint64_t) u, &r(i));
            c = __builtin_add_overflow(r(i+1), c, &r(i+1));
            c |= __builtin_add_overflow(r(i+1), (uint64_t) (u >> 64), &r(i+1));
        }
        r(LIMBS) += o;

        c = o = 0;
        for (size_t i = 0; i < LIMBS; ++i) {
            __uint128_t u = (__uint128_t) y->c[k] * z->c[i];
            o = __builtin_add_overflow(r(i), o, &r(i));
            o |= __builtin_add_overflow(r(i), (uint64_t) u, &r(i));
            c = __builtin_add_overflow(r(i+1), c, &r(i+1));
            c |= __builtin_add_overflow(r(i+1), (uint64_t) (u >> 64), &r(i+1));
        }
        r(LIMBS) += o;
#undef r
    }

    for (size_t i = 0; i < LIMBS; ++i)
        x->c[i] = t[(LIMBS + i) % (LIMBS + 1)];

    reduce_once((uint *) x);
}


void fp_mul2(fp *x, fp const *y)
{
    fp_mul3(x, x, y);
}

void fp_sq2(fp *x, fp const *y)
{
    fp_mul3(x, y, y);
}

void fp_sq1(fp *x)
{
    fp_sq2(x, x);
}

/* (obviously) not constant time in the exponent */
static void fp_pow(fp *x, uint const *e)
{
    fp y = *x;
    *x = fp_1;
    for (size_t k = 0; k < LIMBS; ++k) {
        uint64_t t = e->c[k];
        for (size_t i = 0; i < 64; ++i, t >>= 1) {
            if (t & 1)
                fp_mul2(x, &y);
            fp_sq1(&y);
        }
    }
}

void fp_inv(fp *x)
{
    fp_pow(x, &p_minus_2);
}

bool fp_issquare(fp *x)
{
    fp_pow(x, &p_minus_1_halves);
    return !memcmp(x, &fp_1, sizeof(fp));
}


void fp_random(fp *x)
{
    while (1) {
        randombytes(x, sizeof(fp));
        uint64_t m = ((uint64_t) 1 << pbits % 64) - 1;
        x->c[LIMBS - 1] &= m;

        for (size_t i = LIMBS - 1; i < LIMBS; --i)
            if (x->c[i] < p.c[i])
                return;
            else if (x->c[i] > p.c[i])
                break;
    }
}

