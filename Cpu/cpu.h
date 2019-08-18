/*
 * cpu.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sockets.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "semaforos.h"

#define ENVIO_PROCESO 0
#define FIFO 0
#define RR 1
#define SERIALIZADOR "$$"

typedef struct
{
	int id;
	char* ruta;
	t_list* instrucciones;
	t_list* retornos; //contiene los mensajes de la finalizacion de cada instruccion
					//hay que limpiarlo cada vez que el proceso vuelve a plani
	int iPointer; //instruction pointer
	int quantum;
}proceso;

typedef struct
{
	pthread_t hilo;
	int id;
	int fdPlanif;
	int fdDma;
	t_list* tiempos;
}hiloCpu;

typedef struct
{
	int inicio;
	int fin;
	int diferencia;
}tiempo;

pthread_t hiloPorcentajes;

t_list* hilosCPU;
t_list* tiemposPorc;

int (*ejecutarInstrucciones)(proceso*,hiloCpu*);

char bufferPlanif[256];
char bufferDma[256]; //-> sincronizarlo cuando tengamos varios hilos, o hacer varios buffers

sem_t* mutexBufferPlanif,*mutexBufferDma,*semComandoCpu,*mutex_tActual,*mutexTiem2;


//Convierte los resultados las rafagas a String
char* retornosToString(t_list* retornos);

void crearListas();
void crearHilosCPU();

tiempo* tiem2;
int tActual;
double calcularPorcentaje(hiloCpu* hilo);
char* retornoPorc (double porc, int id);
bool destruirTiemposViejos (tiempo* tiem);
tiempo* createTiempo();
void destroyTiempo(tiempo* tiem);
void tiempoActual(tiempo* tiem);
tiempo* tiempoInicialALista(hiloCpu* hilo);
tiempo* nuevoTiempo(tiempo* tiem1);
bool eliminarTiempoDeLista(tiempo* tiem);
void tiempoFinALista(hiloCpu* hilo, tiempo* tiem1);

void crearSemaforos();
void destruirSemaforos();

proceso* createProceso();
void destroyProceso(proceso* proceso);

hiloCpu* createHiloCpu(int id);
void destroyHiloCpu(hiloCpu* hilo);
void esperarHilo(hiloCpu* hiloCPU);
char* serializarID(int id);

proceso* deserializarPCB (char** serial); //serial contiene id, ruta e instruction pointer separados por "$"
char* serializarProceso (proceso* proc, int tiempoES);	//envia info a plani para que actualice los PCB de los procesos
void setearAlgoritmo(int fdPlanif);

void manejarHilo(hiloCpu* hilo);
void escucharPlani();
void procesarMensajePlanificador(hiloCpu* hilo, char* mensaje);

void conectarPlanificador(hiloCpu* hilo);
void conectarDma(hiloCpu* hilo);

void finalizarCPU(); //llamar al finalzar la CPU, destruye estructuras
void mostrarSerial(char* string); //agrega el "\n" a un string y lo printea

#endif /* CPU_H_ */
