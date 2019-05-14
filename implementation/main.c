
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "fp.h"
#include "csidh.h"
#include "classgroup.h"

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
}

int main()
{
    clock_t t0, t1;
    init_classgroup();

    private_key priv_alice, priv_bob;
    public_key pub_alice, pub_bob;
    public_key shared_alice, shared_bob;

    printf("\n");


    t0 = clock();
    csidh_private(&priv_alice);
    t1 = clock();

    printf("Alice's private key   (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    for (size_t i = 0; i < sizeof(priv_alice); ++i)
        printf("%02hhx", i[(uint8_t *) &priv_alice]);
    printf("\n\n");

    t0 = clock();
    csidh_private(&priv_bob);
    t1 = clock();

    printf("Bob's private key     (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    for (size_t i = 0; i < sizeof(priv_bob); ++i)
        printf("%02hhx", i[(uint8_t *) &priv_bob]);
    printf("\n\n");


    t0 = clock();
    assert(csidh(&pub_alice, &base, &priv_alice));
    t1 = clock();

    printf("Alice's public key    (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    fp_print(&pub_alice.A);
    printf("\n\n");

    t0 = clock();
    assert(csidh(&pub_bob, &base, &priv_bob));
    t1 = clock();

    printf("Bob's public key      (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    fp_print(&pub_bob.A);
    printf("\n\n");


    t0 = clock();
    assert(csidh(&shared_alice, &pub_bob, &priv_alice));
    t1 = clock();

    printf("Alice's shared secret (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    fp_print(&shared_alice.A);
    printf("\n\n");

    t0 = clock();
    assert(csidh(&shared_bob, &pub_alice, &priv_bob));
    t1 = clock();

    printf("Bob's shared secret   (%7.3lf ms):\n  ", 1000. * (t1 - t0) / CLOCKS_PER_SEC);
    fp_print(&shared_bob.A);
    printf("\n\n");

    printf("    ");
    if (memcmp(&shared_alice, &shared_bob, sizeof(public_key)))
        printf("\x1b[31mNOT EQUAL!\x1b[0m\n");
    else
        printf("\x1b[32mequal.\x1b[0m\n");
    printf("\n");

    printf("\n");
    clear_classgroup();
}

