/*
 * adm.h
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#ifndef ADM_H_
#define ADM_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "sockets.h"
#include "semaforos.h"
#include <commons/string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "configuraciones.h"
#include "instruccion.h"
#include "logueos.h"
#include "TLB.h"
#include "marco.h"
#include "estructuras.h"
#include "senales.h"

pthread_t hiloAciertosTLB;

t_list* procesos;
t_list* marcos;
int paginaBuscada;
sem_t* mutexPaginaBuscada,*mutexIdBuscado,*mutexLog,*mutexInstruccion,*mutexNroPagina,*mutexListaMarcos;
sem_t* mutexIdProceso, *mutexNroMarco,*mutexSenales,*mutexTLB;
t_list* TLB;

typedef struct
{
	int fd;
	char* buff;
}t_contexto;

t_pagina *createPagina();
void destroyPagina(t_pagina *self);
void crearPaginas(int cantidad, t_proceso* proceso);
t_pagina* buscarPagina(t_proceso* proceso,int nroPagina);
bool esLaPagina(t_pagina* pagina);

t_proceso *createProceso(int id);
void destroyProceso(t_proceso *self);
bool esElProceso(t_proceso* proceso);
t_proceso* buscarProceso(int id);
int idBuscado;

char* iniciar(t_instruccion* inst);
t_marco* leer(t_instruccion* inst);
t_marco* escribir(t_instruccion* inst);
char* finalizar(t_instruccion* inst);

t_list* crearListaAuxiliar(t_list* marcos);
//solo copia numero de marco y pagina
t_marco* copiarMarco(t_marco* marco);

void dormir();
void setearAlgoritmo();

void crearSemaforos();
void destruirSemaforos();

//puse int para poder salir de la funcion sin recorrer todos los ifs
int procesarMensajeRecibido(t_contexto* contexto);



#endif /* ADM_H_ */
