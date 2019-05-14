
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "params.h"
#include "uint.h"
#include "fp.h"
#include "mont.h"

void xDBLADD(proj *R, proj *S, proj const *P, proj const *Q, proj const *PQ, proj const *A)
{
    fp a, b, c, d;

    fp_add3(&a, &Q->x, &Q->z);
    fp_sub3(&b, &Q->x, &Q->z);
    fp_add3(&c, &P->x, &P->z);
    fp_sub3(&d, &P->x, &P->z);
    fp_sq2(&R->x, &c);
    fp_sq2(&S->x, &d);
    fp_mul2(&c, &b);
    fp_mul2(&d, &a);
    fp_sub3(&b, &R->x, &S->x);
    fp_add3(&a, &A->z, &A->z); /* multiplication by 2 */
    fp_mul3(&R->z, &a, &S->x);
    fp_add3(&S->x, &A->x, &a);
    fp_add2(&R->z, &R->z); /* multiplication by 2 */
    fp_mul2(&R->x, &R->z);
    fp_mul2(&S->x, &b);
    fp_sub3(&S->z, &c, &d);
    fp_add2(&R->z, &S->x);
    fp_add3(&S->x, &c, &d);
    fp_mul2(&R->z, &b);
    fp_sq2(&d, &S->z);
    fp_sq2(&b, &S->x);
    fp_mul3(&S->x, &PQ->z, &b);
    fp_mul3(&S->z, &PQ->x, &d);
}

void xDBL(proj *Q, proj const *A, proj const *P)
{
    fp a, b, c;
    fp_add3(&a, &P->x, &P->z);
    fp_sq1(&a);
    fp_sub3(&b, &P->x, &P->z);
    fp_sq1(&b);
    fp_sub3(&c, &a, &b);
    fp_add2(&b, &b); fp_add2(&b, &b); /* multiplication by 4 */
    fp_mul2(&b, &A->z);
    fp_mul3(&Q->x, &a, &b);
    fp_add3(&a, &A->z, &A->z); /* multiplication by 2 */
    fp_add2(&a, &A->x);
    fp_mul2(&a, &c);
    fp_add2(&a, &b);
    fp_mul3(&Q->z, &a, &c);
}

void xADD(proj *S, proj const *P, proj const *Q, proj const *PQ)
{
    fp a, b, c, d;
    fp_add3(&a, &P->x, &P->z);
    fp_sub3(&b, &P->x, &P->z);
    fp_add3(&c, &Q->x, &Q->z);
    fp_sub3(&d, &Q->x, &Q->z);
    fp_mul2(&a, &d);
    fp_mul2(&b, &c);
    fp_add3(&c, &a, &b);
    fp_sub3(&d, &a, &b);
    fp_sq1(&c);
    fp_sq1(&d);
    fp_mul3(&S->x, &PQ->z, &c);
    fp_mul3(&S->z, &PQ->x, &d);
}

/* Montgomery ladder. */
/* P must not be the unique point of order 2. */
/* not constant-time! */
void xMUL(proj *Q, proj const *A, proj const *P, uint const *k)
{
    proj R = *P;
    const proj Pcopy = *P; /* in case Q = P */

    Q->x = fp_1;
    Q->z = fp_0;

    unsigned long i = 64 * LIMBS;
    while (--i && !uint_bit(k, i));

    do {

        bool bit = uint_bit(k, i);

        if (bit) { proj T = *Q; *Q = R; R = T; } /* not constant-time */
        //fp_cswap(&Q->x, &R.x, bit);
        //fp_cswap(&Q->z, &R.z, bit);

        xDBLADD(Q, &R, Q, &R, &Pcopy, A);

        if (bit) { proj T = *Q; *Q = R; R = T; } /* not constant-time */
        //fp_cswap(&Q->x, &R.x, bit);
        //fp_cswap(&Q->z, &R.z, bit);

    } while (i--);
}

/* computes the isogeny with kernel point K of order k */
/* returns the new curve coefficient A and the image of P */
/* (obviously) not constant time in k */
int xISOG(proj *A, proj *P, proj const *K, uint64_t k, int check)
{
    assert (k >= 3);
    assert (k % 2 == 1);

    fp tmp0, tmp1;
    fp T[4] = {K->z, K->x, K->x, K->z};
    proj Q;

    fp_mul3(&Q.x,  &P->x, &K->x);
    fp_mul3(&tmp0, &P->z, &K->z);
    fp_sub2(&Q.x,  &tmp0);

    fp_mul3(&Q.z,  &P->x, &K->z);
    fp_mul3(&tmp0, &P->z, &K->x);
    fp_sub2(&Q.z,  &tmp0);

    proj M[3] = {*K};
    xDBL(&M[1], A, K);

    uint64_t i;
    for (i = 1; i < k / 2; ++i) {

        if (i >= 2)
            xADD(&M[i % 3], &M[(i - 1) % 3], K, &M[(i - 2) % 3]);

        fp_mul3(&tmp0, &M[i % 3].x, &T[0]);
        fp_mul3(&tmp1, &M[i % 3].z, &T[1]);
        fp_add3(&T[0], &tmp0, &tmp1);

        fp_mul2(&T[1], &M[i % 3].x);

        fp_mul3(&tmp0, &M[i % 3].z, &T[2]);
        fp_mul3(&tmp1, &M[i % 3].x, &T[3]);
        fp_add3(&T[2], &tmp0, &tmp1);

        fp_mul2(&T[3], &M[i % 3].z);


        fp_mul3(&tmp0, &P->x, &M[i % 3].x);
        fp_mul3(&tmp1, &P->z, &M[i % 3].z);
        fp_sub2(&tmp0, &tmp1);
        fp_mul2(&Q.x,  &tmp0);

        fp_mul3(&tmp0, &P->x, &M[i % 3].z);
        fp_mul3(&tmp1, &P->z, &M[i % 3].x);
        fp_sub2(&tmp0, &tmp1);
        fp_mul2(&Q.z,  &tmp0);
    }

    if(check == 1){
        xADD(&M[i % 3], &M[(i - 1) % 3], K, &M[(i - 2) % 3]);

        proj TestPoint;
        xADD(&TestPoint, &M[i % 3], &M[(i - 1) % 3], K);
        
        if (memcmp(&TestPoint.z, &fp_0, sizeof(fp))){
            check = 0;
        } 
    }

    fp_mul2(&T[0], &T[1]);
    fp_add2(&T[0], &T[0]); /* multiplication by 2 */

    fp_sq1(&T[1]);

    fp_mul2(&T[2], &T[3]);
    fp_add2(&T[2], &T[2]); /* multiplication by 2 */

    fp_sq1(&T[3]);

    /* Ax := T[1] * T[3] * Ax - 3 * Az * (T[1] * T[2] - T[0] * T[3]) */
    fp_mul3(&tmp0, &T[1], &T[2]);
    fp_mul3(&tmp1, &T[0], &T[3]);
    fp_sub2(&tmp0, &tmp1);
    fp_mul2(&tmp0, &A->z);
    fp_add3(&tmp1, &tmp0, &tmp0); fp_add2(&tmp0, &tmp1); /* multiplication by 3 */

    fp_mul3(&tmp1, &T[1], &T[3]);
    fp_mul2(&tmp1, &A->x);

    fp_sub3(&A->x, &tmp1, &tmp0);

    /* Az := Az * T[3]^2 */
    fp_sq1(&T[3]);
    fp_mul2(&A->z, &T[3]);

    /* X := X * Xim^2, Z := Z * Zim^2 */
    fp_sq1(&Q.x);
    fp_sq1(&Q.z);
    fp_mul2(&P->x, &Q.x);
    fp_mul2(&P->z, &Q.z);

    return check;
}

/* computes the isogeny with kernel point K of order k */
/* returns the new curve coefficient A and the image of P */
/* (obviously) not constant time in k */
int myxISOG(proj *A, proj *P, int points, proj const *K, uint64_t k, int check)
{
    assert (k >= 3);
    assert (k % 2 == 1);

    fp tmp0, tmp1;
    fp T[4] = {K->z, K->x, K->x, K->z};
    
    proj Q[points];

    for(int p=0; p<points; p++){
        fp_mul3(&Q[p].x,  &P[p].x, &K->x);
        fp_mul3(&tmp0, &P[p].z, &K->z);
        fp_sub2(&Q[p].x,  &tmp0);

        fp_mul3(&Q[p].z,  &P[p].x, &K->z);
        fp_mul3(&tmp0, &P[p].z, &K->x);
        fp_sub2(&Q[p].z,  &tmp0);
    }

    proj M[3] = {*K};
    xDBL(&M[1], A, K);

    uint64_t i;
    for (i = 1; i < k / 2; ++i) {

        if (i >= 2)
            xADD(&M[i % 3], &M[(i - 1) % 3], K, &M[(i - 2) % 3]);

        fp_mul3(&tmp0, &M[i % 3].x, &T[0]);
        fp_mul3(&tmp1, &M[i % 3].z, &T[1]);
        fp_add3(&T[0], &tmp0, &tmp1);

        fp_mul2(&T[1], &M[i % 3].x);

        fp_mul3(&tmp0, &M[i % 3].z, &T[2]);
        fp_mul3(&tmp1, &M[i % 3].x, &T[3]);
        fp_add3(&T[2], &tmp0, &tmp1);

        fp_mul2(&T[3], &M[i % 3].z);

        for(int p=0; p<points ; p++){
            fp_mul3(&tmp0, &P[p].x, &M[i % 3].x);
            fp_mul3(&tmp1, &P[p].z, &M[i % 3].z);
            fp_sub2(&tmp0, &tmp1);
            fp_mul2(&Q[p].x,  &tmp0);

            fp_mul3(&tmp0, &P[p].x, &M[i % 3].z);
            fp_mul3(&tmp1, &P[p].z, &M[i % 3].x);
            fp_sub2(&tmp0, &tmp1);
            fp_mul2(&Q[p].z,  &tmp0);
        }
    }

    if(check == 1){
        xADD(&M[i % 3], &M[(i - 1) % 3], K, &M[(i - 2) % 3]);

        proj TestPoint;
        xADD(&TestPoint, &M[i % 3], &M[(i - 1) % 3], K);
        
        if (memcmp(&TestPoint.z, &fp_0, sizeof(fp))){
            check = 0;
        } 
    }

    fp_mul2(&T[0], &T[1]);
    fp_add2(&T[0], &T[0]); /* multiplication by 2 */

    fp_sq1(&T[1]);

    fp_mul2(&T[2], &T[3]);
    fp_add2(&T[2], &T[2]); /* multiplication by 2 */

    fp_sq1(&T[3]);

    /* Ax := T[1] * T[3] * Ax - 3 * Az * (T[1] * T[2] - T[0] * T[3]) */
    fp_mul3(&tmp0, &T[1], &T[2]);
    fp_mul3(&tmp1, &T[0], &T[3]);
    fp_sub2(&tmp0, &tmp1);
    fp_mul2(&tmp0, &A->z);
    fp_add3(&tmp1, &tmp0, &tmp0); fp_add2(&tmp0, &tmp1); /* multiplication by 3 */

    fp_mul3(&tmp1, &T[1], &T[3]);
    fp_mul2(&tmp1, &A->x);

    fp_sub3(&A->x, &tmp1, &tmp0);

    /* Az := Az * T[3]^2 */
    fp_sq1(&T[3]);
    fp_mul2(&A->z, &T[3]);

    /* X := X * Xim^2, Z := Z * Zim^2 */
    for (int i = 0; i < points; ++i)
    {
        fp_sq1(&Q[i].x);
        fp_sq1(&Q[i].z);
        fp_mul2(&P[i].x, &Q[i].x);
        fp_mul2(&P[i].z, &Q[i].z);
    }

    return check;
}

