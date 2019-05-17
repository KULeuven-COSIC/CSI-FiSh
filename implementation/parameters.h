#ifndef PARAMETERS_H
#define PARAMETERS_H value

//#define MERKLEIZE_PK

#include <openssl/sha.h>
#include "libkeccak.a.headers/SimpleFIPS202.h"

//#define PK_TREE_DEPTH 18
//#define ROUNDS 6
//#define HASHES (1<<14)

//#define PK_TREE_DEPTH 15
//#define ROUNDS 7
//#define HASHES (1<<16)

//#define PK_TREE_DEPTH 12
//#define ROUNDS 9
//#define HASHES (1<<11)

//#define PK_TREE_DEPTH 10
//#define ROUNDS 11
//#define HASHES (1<<7)

#define PK_TREE_DEPTH 8
#define ROUNDS 13
#define HASHES (1<<11)

//#define PK_TREE_DEPTH 6
//#define ROUNDS 16
//#define HASHES (1<<16)

//#define PK_TREE_DEPTH 4
//#define ROUNDS 23
//#define HASHES (1 << 13)

//#define PK_TREE_DEPTH 3
//#define ROUNDS 28
//#define HASHES (1 << 16)

//#define PK_TREE_DEPTH 2
//#define ROUNDS 38
//#define HASHES (1 << 14)

//#define PK_TREE_DEPTH 1
//#define ROUNDS 56
//#define HASHES (1<<16)

#define PKS (1<<PK_TREE_DEPTH)

#define HASH_BYTES 32
#define SEED_BYTES 16

//#define HASH SHA256
#define HASH(data,len,out) SHAKE128(out, HASH_BYTES, data, len);
#define TREEHASH(data,len,out) SHAKE128(out, SEED_BYTES, data, len);
#define EXPAND(data,len,out,outlen) SHAKE128(out, outlen, data, len);

#endif