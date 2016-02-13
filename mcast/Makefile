#
# Makefile
#

LIB = ncurses
CC = gcc

all: mcast

mcast: mcast.c msock.o msock.h
	$(CC) -o mcast mcast.c msock.o

msock: msock.c msock.h
	$(CC) -c msock.c

clean:
	/bin/rm -rf mcast *.o *~ core


