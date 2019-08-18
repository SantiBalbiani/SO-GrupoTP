CC=gcc -v

CFLAGS=-c 


all: Cpu

Cpu: cpu.o configuracion.o logueos.o instrucciones.o 
	$(CC)  cpu.o configuracion.o logueos.o instrucciones.o  -lsharedLibrary -lcommons  -pthread -o Cpu

cpu.o: cpu.c
	$(CC) $(CFLAGS) cpu.c	


configuracion.o: configuracion.c
	 $(CC) $(CFLAGS) configuracion.c 

instrucciones.o: instrucciones.c
	
	 $(CC) $(CFLAGS) instrucciones.c

logueos.o: logueos.c
	$(CC) $(CFLAGS) logueos.c


clean: 
	rm -fr  *.o  Cpu