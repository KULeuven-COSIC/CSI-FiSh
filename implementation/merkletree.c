#include "merkletree.h"

#define LEFT_CHILD(i) (2*i+1)
#define RIGHT_CHILD(i) (2*i+2)
#define PARENT(i) ((i-1)/2)
#define SIBLING(i) (((i)%2)? i+1 : i-1 )
#define IS_LEFT_SIBLING(i) (i%2)

void generate_seed_tree(unsigned char *seed_tree){
	int i;
	for(i=0; i<((1<<PK_TREE_DEPTH)-1); i++){
		EXPAND(seed_tree + i*SEED_BYTES,SEED_BYTES,seed_tree + LEFT_CHILD(i)*SEED_BYTES, 2*SEED_BYTES);
	}
}

void build_tree(const unsigned char *data, int leaf_size, int depth, unsigned char *tree, const unsigned char * merkle_key){
	int i;
	uint16_t buf[9+leaf_size+SEED_BYTES];
	uint16_t *key = buf;
	memcpy(buf+1,merkle_key,SEED_BYTES);
	unsigned char *input = (unsigned char *)(buf+9);

	// hash data to get bottom layer of the tree
	for(i = 0; i<(1<<depth) ; i++){
		memcpy(input,data+i*leaf_size,leaf_size);
		*key = ((1<<depth)-1+i);
		TREEHASH((unsigned char *) buf, leaf_size + 9*sizeof(uint16_t), tree + ((1<<depth)-1+i)*SEED_BYTES);
	}

	// hash the interior of the tree
	for(i = (1<<depth)-2; i >=0; i--)
	{
		memcpy(input,tree + LEFT_CHILD(i)*SEED_BYTES,2*SEED_BYTES);
		*key = i;
		TREEHASH((unsigned char *) buf, 2*SEED_BYTES+ 9*sizeof(uint16_t), tree + i*SEED_BYTES);
	}
}

void get_path(const unsigned char *tree, int depth, int leaf_index, unsigned char *path){
	int i;

	int index = leaf_index+(1<<depth)-1;
	// copy hash values to path
	for (i = 0; i < depth; ++i)
	{
		memcpy(path + i*SEED_BYTES,tree + SIBLING(index)*SEED_BYTES,SEED_BYTES);
		index = PARENT(index);
	}
}

void fill_tree(const unsigned char *indices, unsigned char *tree, int depth){
	int i;

	// 1 = cannot be released
	// 0 = has to be released
	memcpy(tree+(1<<depth)-1,indices, 1<<depth);

	// fill up the internal part of tree
	for(i= (1<<depth)-2; i>=0; i--){
		if((tree[LEFT_CHILD(i)] == 0)  && (tree[RIGHT_CHILD(i)] == 0) ){
			tree[i] = 0;
		}
		else{
			tree[i] = 1;
		}
	}
}

void release_nodes(const unsigned char *tree, int node_size, int depth, unsigned char *indices, unsigned char *out, uint16_t *nodes_released ){
	(*nodes_released) = 0;
	unsigned char *class_tree = calloc(1,(2<<PK_TREE_DEPTH) -1);
	fill_tree(indices,class_tree,depth);

	int i;
	for(i=0; i<(2<<depth)-1; i++){
		if((class_tree[i] == 0) && (class_tree[PARENT(i)] == 1)){
			memcpy(out + ((*nodes_released)++)*node_size, tree + i*node_size, node_size);
		}
	}
	free(class_tree);
}

void hash_up(const uint *data, unsigned char *indices, const unsigned char *in, int in_len, unsigned char *root, const unsigned char * merkle_key){
	unsigned char *tree = calloc(1,((2<<PK_TREE_DEPTH)-1)*SEED_BYTES);
	uint16_t buf[9+sizeof(uint)+SEED_BYTES];
	uint16_t *key = buf;
	memcpy(buf+1,merkle_key,SEED_BYTES);
	unsigned char *input = (unsigned char *)(buf+9);
	*key = 0;

	unsigned char class_tree[(2<<PK_TREE_DEPTH) -1] = {0};
	fill_tree(indices,class_tree,PK_TREE_DEPTH);

	int i;
	int nodes_not_used = in_len;
	// hash data to get bottom layer of the tree
	for(i = ((2<<PK_TREE_DEPTH) -2) ; i>=((1<<PK_TREE_DEPTH) -1) ; i--){
		if(class_tree[i] == 1){
			memcpy(input,(unsigned char *)(data+ (i-((1<<PK_TREE_DEPTH) -1))),sizeof(uint));
			*key = i; 
			TREEHASH((unsigned char *) buf, sizeof(uint)+ 9*sizeof(uint16_t), tree + i*SEED_BYTES);
		} 
		else if (class_tree[SIBLING(i)] == 1){
			memcpy(tree + i*SEED_BYTES , in + SEED_BYTES*(--nodes_not_used), SEED_BYTES );
		}
	}

	// hash the interior of the tree
	for(i = (1<<PK_TREE_DEPTH)-2; i >=0; i--)
	{
		if(class_tree[i] == 1){
			memcpy(input,tree + LEFT_CHILD(i)*SEED_BYTES, 2*SEED_BYTES);
			*key = i;
			TREEHASH((unsigned char *) buf, 2*SEED_BYTES+9*sizeof(uint16_t), tree + i*SEED_BYTES);
		}
		else if( i>0 && class_tree[SIBLING(i)] ==  1 ){
			memcpy(tree + i*SEED_BYTES , in + SEED_BYTES*(--nodes_not_used), SEED_BYTES );
		}
	}

	memcpy(root,tree,SEED_BYTES);
	free(tree);
}

void print_seed(const unsigned char *seed){
	int i=0;
	for(i=0; i<SEED_BYTES ; i++){
		printf("%2X ", seed[i]);
	}
	printf("\n");
}

void print_TREEHASH(const unsigned char *hash){
	int i=0;
	for(i=0; i<SEED_BYTES ; i++){
		printf("%2X ", hash[i]);
	}
	printf("\n");
}

void print_tree(const unsigned char *tree, int depth){
	int i=0;
	for(i=0; i<(2<<depth)-1 ; i++){
		printf("%4d: ", i);
		print_TREEHASH(tree + SEED_BYTES*i);
	}
	printf("\n");
}