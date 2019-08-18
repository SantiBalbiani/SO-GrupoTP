CC=gcc -v

CFLAGS=-c 


all: Planificador

Planificador: planificador.o configuracion.o logueos.o pcb.o consola.o 
	$(CC)  planificador.o configuracion.o logueos.o pcb.o consola.o  -lsharedLibrary -lcommons  -pthread -o Planificador

planificador.o: planificador.c
	$(CC) $(CFLAGS) planificador.c	


configuracion.o: configuracion.c
	 $(CC) $(CFLAGS) configuracion.c 

pcb.o: pcb.c
	$(CC) $(CFLAGS) pcb.c

consola.o: consola.c
	
	 $(CC) $(CFLAGS) consola.c

logueos.o: logueos.c
	$(CC) $(CFLAGS) logueos.c


clean: 
	rm -fr  *.o  Planificador


