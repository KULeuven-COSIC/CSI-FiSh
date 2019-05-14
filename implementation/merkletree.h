#ifndef MERKLETREE_H
#define MERKLETREE_H

#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include <string.h>
#include "parameters.h"
#include "csidh.h"

void generate_seed_tree(unsigned char *seed_tree);

void build_tree(const unsigned char *data,int leaf_size, int depth, unsigned char *tree, const unsigned char *merkle_key);
void get_path(const unsigned char *tree, int depth, int leaf_index, unsigned char *path);
void follow_path(const unsigned char *leaf, int leaf_size, int depth, const unsigned char *path, int index, unsigned char *root, const unsigned char *merkle_key);

void release_nodes(const unsigned char *tree, int node_size, int depth, unsigned char *indices, unsigned char *out, uint16_t *nodes_released );

void print_seed(const unsigned char *seed);
void print_hash(const unsigned char *hash);
void print_tree(const unsigned char *tree, int depth);

void hash_up(const uint *data, unsigned char *indices, const unsigned char *in, int in_len, unsigned char *root, const unsigned char *merkle_key);

#endif