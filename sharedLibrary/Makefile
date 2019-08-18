CC=gcc 
LIBS=-lcommons
CFLAGS=-c -fPIC

all: sharedLibrary.so

sharedLibrary.so: sockets.o semaforos.o
	$(CC) -shared -o "libsharedLibrary.so" sockets.o semaforos.o $(LIBS)	

semaforos.o: semaforos.c
	$(CC) $(CFLAGS) $(LIBS) semaforos.c

sockets.o: sockets.c
	$(CC) $(CFLAGS) $(LIBS) sockets.c

install:
	cp -vf "libsharedLibrary.so" /usr/lib/
	cp -vf sockets.h semaforos.h /usr/include
clean:
	rm -fr *.o "libsharedLibrary.so"



