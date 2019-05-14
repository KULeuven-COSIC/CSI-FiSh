#ifndef MONT_H
#define MONT_H

#include "params.h"

void xDBL(proj *Q, proj const *A, proj const *P);
void xADD(proj *S, proj const *P, proj const *Q, proj const *PQ);
void xDBLADD(proj *R, proj *S, proj const *P, proj const *Q, proj const *PQ, proj const *A);
void xMUL(proj *Q, proj const *A, proj const *P, uint const *k);
int xISOG(proj *A, proj *P, proj const *K, uint64_t k, int check);
int myxISOG(proj *A, proj *P, int points, proj const *K, uint64_t k, int check);

#endif
