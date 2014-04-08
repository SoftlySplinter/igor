CC=cc
CFLAGS=-std=c99 -Wall
LDFLAGS=-lm -ledit
OUT=bin

all: ${OUT}
	${CC} ${CFLAGS} igor.c lib/mpc.c ${LDFLAGS} -o ${OUT}/igor

${OUT}:
	mkdir ${OUT}

clean:
	rm ${OUT}/igor
	rmdir ${OUT}
