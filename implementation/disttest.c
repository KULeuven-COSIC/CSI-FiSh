
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "classgroup.h"
#include "csidh.h"

#define SAMPLES 10000
#define NUM_PRIMES 74

#define max(A,B) ((A>B)? A:B)

static inline
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

int main(){
	init_classgroup();

	double l1 = 0;
	double l2 = 0;
	double linf = 0;
	double l1sq = 0;
	double l2sq = 0;
	double linfsq = 0;
	double time = 0;
	double timesq = 0;

	for (int i = 0; i < SAMPLES; ++i)
	{
		public_key pub = {{{0}}};

		uint64_t t = rdtsc();
		
			private_key priv;
			csidh_private(&priv);
			
			if(i%10 == 0)
				action(&pub, &pub, &priv);

		t = rdtsc() - t;

		int L1 = 0, L2 = 0, Linf = 0;
		for(int j=0; j<NUM_PRIMES; j++){
			L1 += abs(priv.e[j]);
			L2 += priv.e[j]*priv.e[j];
			Linf = max(Linf,abs(priv.e[j]));
		}

		l1 += L1;
		l2 += sqrt((double) L2);
		linf += Linf;

		l1sq += L1*L1;
		l2sq += L2;
		linfsq += Linf*Linf;

		if(i%10 == 0){
			time += t;
			timesq += t*t;
		}
	}

	l1 /=SAMPLES;
	l2 /=SAMPLES;
	linf /=SAMPLES;
	time /=(SAMPLES/10);
	l1sq /=SAMPLES;
	l2sq /=SAMPLES;
	linfsq /=SAMPLES;
	timesq /=(SAMPLES/10);

	l1sq -= l1*l1;
	l2sq -= l2*l2;
	linfsq -= linf*linf;
	timesq -= time*time;

	printf("l1   = %lf +- %lf\n", l1 , sqrt(l1sq) );
	printf("l2   = %lf +- %lf\n", l2 , sqrt(l2sq) );
	printf("linf = %lf +- %lf\n", linf , sqrt(linfsq) );
	printf("time = %lf +- %lf\n", time/1000000.0 , sqrt(timesq)/1000000.0 );

}