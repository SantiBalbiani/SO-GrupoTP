CC=gcc

CFLAGS=-v -c

LIBS=-lsharedLibrary -lcommons -pthread 

all: Adm

Adm: adm.o configuraciones.o  instruccion.o algoritmos.o  conexiones.o TLB.o senales.o marco.o logueos.o
	$(CC)  adm.o configuraciones.o instruccion.o algoritmos.o conexiones.o TLB.o senales.o marco.o logueos.o $(LIBS) -o Adm

adm.o: adm.c
	$(CC) $(CFLAGS) adm.c	

instruccion.o: instruccion.c
	 $(CC) $(CFLAGS) instruccion.c

algoritmos.o: algoritmos.c
	 $(CC) $(CFLAGS) algoritmos.c

conexiones.o: conexiones.c
	$(CC) $(CFLAGS) conexiones.c

TLB.o: TLB.c
	$(CC) $(CFLAGS) TLB.c

senales.o: senales.c
	$(CC) $(CFLAGS) senales.c

configuraciones.o: configuraciones.c
	 $(CC) $(CFLAGS) configuraciones.c 

logueos.o: logueos.c
	$(CC) $(CFLAGS) logueos.c

marco.o: marco.c
	
	 $(CC) $(CFLAGS) marco.c

clean: 
	rm -fr  *.o  Adm


