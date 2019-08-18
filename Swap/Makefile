CC=gcc -v

CFLAGS=-c


all: Swap

Swap: swap.o
	$(CC)  swap.o  -lsharedLibrary -lcommons  -pthread -o Swap

swap.o: swap.c
	$(CC) $(CFLAGS) swap.c	

clean: 
	rm -fr  *.o  Swap