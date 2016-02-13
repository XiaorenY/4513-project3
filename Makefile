#
# Makefile
#

LIB = ncurses
CC = gcc

all: nutella

nutella: nutella.c msock.o msock.h
	$(CC) -o nutella nutella.c msock.o

msock: msock.c msock.h
	$(CC) -c msock.c

clean:
	/bin/rm -rf nutella *.o *~ core


