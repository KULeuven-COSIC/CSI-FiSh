
#include "reduce.h"
#include "params.h"

#define K 74
#define K2 K
#define ITERATIONS 10
#define TRIALS 100

#include "pool.c"

int32_t rel_lat[K*K];

int32_t l1norm( int32_t *vec ){
	int32_t sum = 0;
	for(int i=0; i<K2 ; i++){
		sum += abs(vec[i]);
	}
	return sum;
}

int32_t l1normsum( int32_t *vec1, int32_t *vec2 ){
	int32_t sum = 0;
	for(int i=0; i<K2 ; i++){
		sum += abs(vec1[i]+vec2[i]);
	}
	return sum;
}

int32_t l1normdif( int32_t *vec1, int32_t *vec2 ){
	int32_t sum = 0;
	for(int i=0; i<K2 ; i++){
		sum += abs(vec1[i]-vec2[i]);
	}
	return sum;
}

int32_t l2norm( int32_t *vec ){
	int32_t sum = 0;
	for(int i=0; i<K ; i++){
		sum += vec[i]*vec[i];
	}
	return sum;
}

int32_t l2normsum( int32_t *vec1, int32_t *vec2 ){
	int32_t sum = 0;
	for(int i=0; i<K ; i++){
		sum += (vec1[i]+vec2[i])*(vec1[i]+vec2[i]);
	}
	return sum;
}

int32_t l2normdif( int32_t *vec1, int32_t *vec2 ){
	int32_t sum = 0;
	for(int i=0; i<K ; i++){
		sum += (vec1[i]-vec2[i])*(vec1[i]-vec2[i]);
	}
	return sum;
}

void addvec( int32_t *vec1, int32_t *vec2 ){
	for(int i=0; i<K ; i++){
		vec1[i]+=vec2[i];
	}
}

void subvec( int32_t *vec1, int32_t *vec2 ){
	for(int i=0; i<K ; i++){
		vec1[i]-=vec2[i];
	}
}

void reduce_32(int32_t *vec, int pool_vectors){
	int32_t norm = l1norm(vec);
	int i;
	int counter = 0;
	while (1){
		int change = 0;
		for(i=0; i<pool_vectors; i++){
			int32_t plus_norm = l1normsum(vec,pool + i*K);
			if(plus_norm < norm){
				norm = plus_norm;
				counter ++;
				addvec(vec,pool +i*K);
				change = 1;
			} 
			int32_t minus_norm = l1normdif(vec,pool + i*K);
			if(minus_norm < norm){
				norm = minus_norm;
				counter ++;
				subvec(vec,pool +i*K);
				change = 1;
			}
		}
		if (change == 0){
			return;
		}
	}
}

void reduce(int8_t *vec, int trials, int pool_vectors){
	int32_t VEC[K];
	for(int i=0; i<K; i++){
		VEC[i] = (int32_t) vec[i];
	}

	int32_t best[K];
	int32_t best_len;

	reduce_32(VEC,pool_vectors);

	memcpy(best,VEC,sizeof(int32_t)*K);
	best_len = l1norm(VEC);

	for(int i=1 ; i<trials; i++){
		memcpy(VEC,best,sizeof(int32_t)*K);

		for(int j=0; j<2 ; j++){
			int index = rand()%POOL_SIZE;
			addvec(VEC,pool+K*index);
		}

		reduce_32(VEC,pool_vectors);
		int norm = l1norm(VEC);

		if(norm < best_len){
			best_len = norm;
			memcpy(best,VEC,sizeof(int32_t)*K);
		}

	}

	for(int i=0; i<K; i++){
		vec[i] = (int8_t) best[i];
	}
}
