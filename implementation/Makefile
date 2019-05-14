
BITS?=512

ifndef UINT_IMPL
	UINT_IMPL=uint.c
	ifneq ("$(wildcard p${BITS}/uint.s)", "")
		UINT_IMPL="$(wildcard p${BITS}/uint.*)"
	endif
endif

ifndef FP_IMPL
	FP_IMPL=fp.c
	ifneq ("$(wildcard p${BITS}/fp.s)", "")
		FP_IMPL="$(wildcard p${BITS}/fp.*)"
	endif
endif



all:
	@cc \
		-I ./ \
		-I p${BITS}/ \
		-std=c99 -pedantic \
		-Wall -Wextra \
		-march=native -O3 \
		p${BITS}/constants.c \
		rng.c \
		${UINT_IMPL} ${FP_IMPL} \
		mont.c \
		classgroup.c \
		csidh.c \
		main.c \
		reduce.c \
		-o main -lm -lgmp -lcrypto

debug:
	cc \
		-I ./ \
		-I p${BITS}/ \
		-std=c99 -pedantic \
		-Wall -Wextra \
		-g \
		p${BITS}/constants.c \
		rng.c \
		${UINT_IMPL} ${FP_IMPL} \
		mont.c \
		csidh.c \
		main.c \
		-o main -lm -lcrypto

testcsifish: csidh.c mont.c classgroup.c reduce.c reduce.h test.c csidh.h mont.h classgroup.h keccaklib merkletree.c merkletree.h
	@cc \
		$(if ${BENCH_ITS},-DBENCH_ITS=${BENCH_ITS}) \
		$(if ${BENCH_VAL},-DBENCH_VAL=${BENCH_VAL}) \
		$(if ${BENCH_ACT},-DBENCH_ACT=${BENCH_ACT}) \
		-I ./ \
		-I p${BITS}/ \
		-I XKCP/bin/generic64/ \
		-L XKCP/bin/generic64/ \
		-std=c11 -pedantic \
		-Wall -Wextra \
		-march=native -O3 \
		p${BITS}/constants.c \
		rng.c \
		${UINT_IMPL} ${FP_IMPL} \
		mont.c \
		csidh.c \
		test.c\
		csifish.c \
		merkletree.c \
		reduce.c \
		classgroup.c \
		-o testcsifish -lm -g -lgmp -lcrypto -lkeccak

bench: csidh.c bench.c mont.c classgroup.c reduce.c reduce.h keccaklib
	@cc \
		$(if ${BENCH_ITS},-DBENCH_ITS=${BENCH_ITS}) \
		$(if ${BENCH_VAL},-DBENCH_VAL=${BENCH_VAL}) \
		$(if ${BENCH_ACT},-DBENCH_ACT=${BENCH_ACT}) \
		-I ./ \
		-I p${BITS}/ \
		-I XKCP/bin/generic64/ \
		-L XKCP/bin/generic64/ \
		-std=c99 -pedantic \
		-Wall -Wextra \
		-march=native -O3 \
		p${BITS}/constants.c \
		rng.c \
		${UINT_IMPL} ${FP_IMPL} \
		mont.c \
		csidh.c \
		bench.c \
		parameters.h \
		reduce.c \
		classgroup.c \
		-o bench -lm -g -lgmp -lcrypto -lkeccak

disttest: disttest.c classgroup.c classgroup.h reduce.c reduce.h keccaklib csidh.c csidh.h
	@cc \
		-I ./ \
		-I p${BITS}/ \
		-I XKCP/bin/generic64/ \
		-L XKCP/bin/generic64/ \
		-std=c11 -pedantic \
		-Wall -Wextra \
		-march=native -O3 \
		p${BITS}/constants.c \
		rng.c \
		${UINT_IMPL} ${FP_IMPL} \
		disttest.c\
		csidh.c \
		mont.c \
		reduce.c \
		classgroup.c \
		-o disttest -lm -g -lgmp -lcrypto -lkeccak

keccaklib: 
	(cd XKCP; make generic64/libkeccak.a)

clean:
	@rm -f main bench testcsifish

