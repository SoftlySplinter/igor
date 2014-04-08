CC=cc
CFLAGS=-std=c99 -Wall

all:
	${CC} ${CFLAGS} igor.c lib/mpc.c -lm -ledit -o igor
