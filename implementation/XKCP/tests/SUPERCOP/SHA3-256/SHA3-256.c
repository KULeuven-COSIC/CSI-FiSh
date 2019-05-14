/*
Implementation by the Keccak Team, namely, Guido Bertoni, Joan Daemen,
Michaël Peeters, Gilles Van Assche and Ronny Van Keer,
hereby denoted as "the implementer".

For more information, feedback or questions, please refer to our website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#include "crypto_hash.h"
#ifndef crypto_hash_BYTES
    #define crypto_hash_BYTES 32
#endif
#include "KeccakSpongeWidth1600.h"

int crypto_hash( unsigned char *out, const unsigned char *in, unsigned long long inlen )
{
    return KeccakWidth1600_Sponge(1088, 512, in, inlen, 0x06, out, crypto_hash_BYTES);
}
