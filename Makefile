
CC = gcc
CFLAGS = -g -Werror -Wall

.PHONY: all

all: cargounpack cargolist cargopack

cargounpack: utils.o vfs.o cargounpack.o
	${CC} ${CFLAGS} -o $@ cargounpack.o utils.o vfs.o

cargolist: utils.o vfs.o cargolist.o
	${CC} ${CFLAGS} -o $@ cargolist.o utils.o vfs.o

cargopack: utils.o vfs.o cargopack.o
	${CC} ${CFLAGS} -o $@ cargopack.o utils.o vfs.o
