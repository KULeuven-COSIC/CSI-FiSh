
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <inttypes.h>

#include "rng.h"
#include "csidh.h"
#include "classgroup.h"
#include "fp.h"
#include "reduce.h"

static inline
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#define TIC printf("\n"); uint64_t cl = rdtsc();
#define TOC(A) printf("%s cycles = %lu \n",#A ,rdtsc() - cl); cl = rdtsc();


/* defaults */

#ifndef BENCH_ITS
    #define BENCH_ITS 500
#endif

#if !defined(BENCH_VAL) && !defined(BENCH_ACT)
    #define BENCH_VAL 1
    #define BENCH_ACT 1
#endif
#ifndef BENCH_VAL
    #define BENCH_VAL 0
#endif
#ifndef BENCH_ACT
    #define BENCH_ACT 0
#endif


const unsigned long its = BENCH_ITS;
const bool val = BENCH_VAL, act = BENCH_ACT;


const size_t stacksz = 0x8000;  /* 32k */

/* csidh.c */
bool validate(public_key const *in);
void action(public_key *out, public_key const *in, private_key const *priv);

void uint_print(uint const *x)
{
    for (size_t i = 8*LIMBS-1; i < 8*LIMBS; --i)
        printf("%02hhx", i[(unsigned char *) x->c]);
}

void fp_print(fp const *x)
{
    uint y;
    fp_dec(&y, x);
    uint_print(&y);
    printf("\n");
}

int main()
{
    clock_t t0, t1, time = 0;
    uint64_t c0, c1, cycles = 0;
    size_t bytes = 0;
    unsigned char *stack;

    init_classgroup();

    private_key priv;
    public_key pub = {{{0}}};
    (void) pub; /* suppress "unused variable" warning */

    //pub.A.c[0]++;

    printf("doing %lu iterations of%s%s%s.\n",
        its,
        val ? " validation" : "",
        val && act ? " and" : !val && !act ? " nothing" : "",
        act ? " action" : ""
    );

    __asm__ __volatile__ ("mov %%rsp, %0" : "=m"(stack));
    stack -= stacksz;

    for (unsigned long i = 0; i < its; ++i) {

        public_key pub = {{{0}}};

        if (its >= 100 && i % (its / 100) == 0) {
            printf("%2lu%%", 100 * i / its);
            fflush(stdout);
            printf("\r\x1b[K");
        }

        /* spray stack */
        unsigned char canary;
        randombytes(&canary, 1);
        for (size_t j = 0; j < stacksz; ++j)
            stack[j] = canary;

        t0 = clock();   /* uses stack, but not too much */
        c0 = rdtsc();

        /**************************************/ 

        csidh_private(&priv);

        if (val)
            assert(validate(&pub));

        if (act)
            action(&pub, &pub, &priv);


        /**************************************/

        c1 = rdtsc();
        t1 = clock();
        cycles += c1 - c0;
        time += t1 - t0;

        /* check stack */
        assert(*stack == canary);   /* make sure we sprayed enough */
        for (size_t j = 0; j < stacksz - bytes; ++j)
            if (stack[j] != canary)
                bytes = stacksz - j;
    }

    printf("iterations: %lu\n", its);
    printf("clock cycles: %" PRIu64 " (%.1lf*10^6)\n", (uint64_t) cycles / its, 1e-6 * cycles / its);
    printf("wall-clock time: %.3lf ms\n", 1000. * time / CLOCKS_PER_SEC / its);
    printf("stack memory usage: %lu b\n", bytes);

    clear_classgroup();
}

