
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "uint.h"
#include "fp.h"
#include "mont.h"
#include "csidh.h"
#include "rng.h"
#include "classgroup.h"

//#define ORIGINAL
#define UNIFORM

const public_key base = {{{0}}}; /* A = 0 */

#ifdef UNIFORM
void csidh_private(private_key *priv)
{
    sample_from_classgroup(priv->e);
}
#endif

#ifdef ORIGINAL
#define MAX_EXPONENT 5
/* TODO allow different encodings depending on parameters */
/* TODO waste less randomness */
void csidh_private(private_key *priv)
{
    memset(&priv->e, 0, sizeof(priv->e));
    for (size_t i = 0; i < NUM_PRIMES; ) {
        int8_t buf[64];
        randombytes(buf, sizeof(buf));
        for (size_t j = 0; j < sizeof(buf); ++j) {
            if (buf[j] <= MAX_EXPONENT && buf[j] >= -MAX_EXPONENT) {
                priv->e[i] = buf[j];
                if (++i >= NUM_PRIMES)
                    break;
            }
        }
    }
}
#endif

static bool validate_rec(proj *P, proj const *A, size_t lower, size_t upper, uint *order, bool *is_supersingular)
{
    assert(lower < upper);

    if (upper - lower == 1) {

        /* now P is [(p+1) / l_lower] times the original random point */
        /* we only gain information if this multiple is non-zero */

        if (memcmp(&P->z, &fp_0, sizeof(fp))) {

            uint tmp;
            uint_set(&tmp, primes[lower]);
            xMUL(P, A, P, &tmp);

            if (memcmp(&P->z, &fp_0, sizeof(fp))) {
                /* order does not divide p+1. */
                *is_supersingular = false;
                return true;
            }

            uint_mul3_64(order, order, primes[lower]);

            if (uint_sub3(&tmp, &four_sqrt_p, order)) { /* returns borrow */
                /* order > 4 sqrt(p), hence definitely supersingular */
                *is_supersingular = true;
                return true;
            }
        }

        /* inconclusive */
        return false;
    }

    size_t mid = lower + (upper - lower + 1) / 2;

    uint cl = uint_1, cu = uint_1;
    for (size_t i = lower; i < mid; ++i)
        uint_mul3_64(&cu, &cu, primes[i]);
    for (size_t i = mid; i < upper; ++i)
        uint_mul3_64(&cl, &cl, primes[i]);

    proj Q;

    xMUL(&Q, A, P, &cu);
    xMUL(P, A, P, &cl);

    /* start with the right half; bigger primes help more */
    return validate_rec(&Q, A, mid, upper, order, is_supersingular)
        || validate_rec(P, A, lower, mid, order, is_supersingular);
}

/* never accepts invalid keys. */
bool validate(public_key const *in)
{
    /* make sure the curve is nonsingular: A^2-4 != 0 */
    {
        uint dummy;
        if (!uint_sub3(&dummy, (uint *) &in->A, &p)) /* returns borrow */
            /* A >= p */
            return false;

        fp fp_pm2;
        fp_set(&fp_pm2, 2);
        if (!memcmp(&in->A, &fp_pm2, sizeof(fp)))
            /* A = 2 */
            return false;

        fp_sub3(&fp_pm2, &fp_0, &fp_pm2);
        if (!memcmp(&in->A, &fp_pm2, sizeof(fp)))
            /* A = -2 */
            return false;
    }

    const proj A = {in->A, fp_1};

    do {
        proj P;
        fp_random(&P.x);
        P.z = fp_1;

        /* maximal 2-power in p+1 */
        xDBL(&P, &A, &P);
        xDBL(&P, &A, &P);

        bool is_supersingular;
        uint order = uint_1;

        if (validate_rec(&P, &A, 0, NUM_PRIMES, &order, &is_supersingular))
            return is_supersingular;

    /* P didn't have big enough order to prove supersingularity. */
    } while (1);
}

/* compute x^3 + Ax^2 + x */
static void montgomery_rhs(fp *rhs, fp const *A, fp const *x)
{
    fp tmp;
    *rhs = *x;
    fp_sq1(rhs);
    fp_mul3(&tmp, A, x);
    fp_add2(rhs, &tmp);
    fp_add2(rhs, &fp_1);
    fp_mul2(rhs, x);
}

static __inline__ uint64_t rdtsc(void)
{
    uint32_t hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return lo | (uint64_t) hi << 32;
}

/* totally not constant-time. */
void action(public_key *out, public_key const *in, private_key const *priv)
{
    uint k[2];
    uint_set(&k[0], 4); /* maximal 2-power in p+1 */
    uint_set(&k[1], 4); /* maximal 2-power in p+1 */

    uint8_t e[2][NUM_PRIMES];

    for (size_t i = 0; i < NUM_PRIMES; ++i) {

        int8_t t = (int8_t) priv->e[i] ;

        if (t > 0) {
            e[0][i] = t;
            e[1][i] = 0;
            uint_mul3_64(&k[1], &k[1], primes[i]);
        }
        else if (t < 0) {
            e[1][i] = -t;
            e[0][i] = 0;
            uint_mul3_64(&k[0], &k[0], primes[i]);
        }
        else {
            e[0][i] = 0;
            e[1][i] = 0;
            uint_mul3_64(&k[0], &k[0], primes[i]);
            uint_mul3_64(&k[1], &k[1], primes[i]);
        }
    }

    proj A = {in->A, fp_1};

    bool done[2] = {false, false};

    int count = 0;

    do {

        assert(!memcmp(&A.z, &fp_1, sizeof(fp)));

        proj P;
        fp_random(&P.x);
        P.z = fp_1;

        fp rhs;
        montgomery_rhs(&rhs, &A.x, &P.x);
        bool sign = !fp_issquare(&rhs);

        if (done[sign])
            continue;
        
        count ++;

        //uint64_t T = rdtsc();

        xMUL(&P, &A, &P, &k[sign]);

        /*printf("%d , %d \n",count , rdtsc() - T);
        for(int l=0; l<NUM_PRIMES ; l++){
            printf("%2d", e[0][l]-e[1][l]);
        }
        printf("\n");*/

        done[sign] = true;

        for (size_t i = NUM_PRIMES - 1; i < NUM_PRIMES; --i) {

            if (e[sign][i]) {

                uint cof = uint_1;
                for (size_t j = 0; j < i; ++j)
                    if (e[sign][j])
                        uint_mul3_64(&cof, &cof, primes[j]);

                proj K;
                xMUL(&K, &A, &P, &cof);

                if (memcmp(&K.z, &fp_0, sizeof(fp))) {                    

                    // T = rdtsc();

                    xISOG(&A, &P, &K, primes[i],0);

                    //printf("   i:%2d cyc:%d cyc/p:%d \n", i, rdtsc() - T, (rdtsc() - T)/primes[i]);

                    if (!--e[sign][i])
                        uint_mul3_64(&k[sign], &k[sign], primes[i]);

                }

            }

            done[sign] &= !e[sign][i];
        }

        fp_inv(&A.z);
        fp_mul2(&A.x, &A.z);
        A.z = fp_1;

    } while (!(done[0] && done[1]));

    //printf("\n");

    out->A = A.x;

}

/* includes public-key validation. */
bool csidh(public_key *out, public_key const *in, private_key const *priv)
{
    if (!validate(in)) {
        fp_random(&out->A);
        return false;
    }
    action(out, in, priv);
    return true;
}

