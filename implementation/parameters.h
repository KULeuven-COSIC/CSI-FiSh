#ifndef PARAMETERS_H
#define PARAMETERS_H value

//#define MERKLEIZE_PK

#include <openssl/sha.h>
#include "libkeccak.a.headers/SimpleFIPS202.h"

//#define PK_TREE_DEPTH 19
//#define ROUNDS 6
//#define HASHES (1<<14)

//#define PK_TREE_DEPTH 16
//#define ROUNDS 7
//#define HASHES (1<<16)

//#define PK_TREE_DEPTH 12
//#define ROUNDS 10
//#define HASHES (1<<8)

#define PK_TREE_DEPTH 8
#define ROUNDS 14
#define HASHES (1<<16)

//#define PK_TREE_DEPTH 4
//#define ROUNDS 28
//#define HASHES (1 << 16)

//#define PK_TREE_DEPTH 1
//#define ROUNDS 111
//#define HASHES (1<<17)

#define PKS (1<<PK_TREE_DEPTH)

#define HASH_BYTES 32
#define SEED_BYTES 16

//#define HASH SHA256
#define HASH(data,len,out) SHAKE128(out, HASH_BYTES, data, len);
#define TREEHASH(data,len,out) SHAKE128(out, SEED_BYTES, data, len);
#define EXPAND(data,len,out,outlen) SHAKE128(out, outlen, data, len);

#endif