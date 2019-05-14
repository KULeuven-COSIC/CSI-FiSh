/*
Implementation by Ronny Van Keer, hereby denoted as "the implementer".

For more information, feedback or questions, please refer to our website:
https://keccak.team/

To the extent possible under law, the implementer has waived all copyright
and related or neighboring rights to the source code in this file.
http://creativecommons.org/publicdomain/zero/1.0/

---

Please refer to SnP-documentation.h for more details.
*/

#ifndef _KeccakP_800_SnP_h_
#define _KeccakP_800_SnP_h_

#define KeccakP800_implementation      "8-bit optimized AVR assembler implementation"
#define KeccakP800_stateSizeInBytes    100
#define KeccakP800_stateAlignment      8

void KeccakP800_StaticInitialize( void );
/* #define   KeccakP800_StaticInitialize() */
void KeccakP800_Initialize(void *state);
void KeccakP800_AddByte(void *state, unsigned char data, unsigned int offset);
/* #define   KeccakP800_AddByte(argS, argData, argOffset)    ((unsigned char*)argS)[argOffset] ^= (argData) */
void KeccakP800_AddBytes(void *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP800_OverwriteBytes(void *state, const unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP800_OverwriteWithZeroes(void *state, unsigned int byteCount);
void KeccakP800_Permute_Nrounds(void *state, unsigned int nrounds);
void KeccakP800_Permute_12rounds(void *state);
void KeccakP800_Permute_22rounds(void *state);
void KeccakP800_ExtractBytes(const void *state, unsigned char *data, unsigned int offset, unsigned int length);
void KeccakP800_ExtractAndAddBytes(const void *state, const unsigned char *input, unsigned char *output, unsigned int offset, unsigned int length);

#endif
