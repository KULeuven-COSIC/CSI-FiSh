/*
Implementation by Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to our website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/
*/

#ifdef KeccakReference
    #include "displayIntermediateValues.h"
#endif

#include "KetMj.h"
#include "KetjeMj.h"

#ifndef KeccakP1600_excluded
    #include "KeccakP-1600-SnP.h"

    #define prefix                      KetjeMj
    #define prefixKet                   KetMj
    #define SnP                         KeccakP1600
    #define SnP_width                   1600
    #define SnP_PermuteRounds           KeccakP1600_Permute_Nrounds
        #include "Ketjev2.inc"
    #undef prefix
    #undef prefixKet
    #undef SnP
    #undef SnP_width
    #undef SnP_PermuteRounds
#endif
