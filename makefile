CC=cc
CFLAGS=-std=c99 -Wall
LDFLAGS=lib/mpc.c -lm -ledit
OUT=bin
SRC=src

all: ${OUT} igor

igor: parse
	${CC} ${CFLAGS} ${SRC}/igor.c ${OUT}/parse.o ${LDFLAGS} -o ${OUT}/igor

parse:
	${CC} ${CFLAGS} ${SRC}/parse.c -c -o ${OUT}/parse.o

${OUT}:
	mkdir ${OUT}

clean:
	rm ${OUT}/igor
	rmdir ${OUT}
