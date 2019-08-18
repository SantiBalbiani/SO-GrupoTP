/*
 * logueos.h
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */

#ifndef LOGUEOS_H_
#define LOGUEOS_H_
#include <stdbool.h>
#include <commons/log.h>
#include "instrucciones.h"
#include "semaforos.h"

t_log *logCPU;
sem_t* mutexLog;

void crearLog();
void destruirLog();

void loguearCPUConectada(int id); //id de la cpu, TESTEAR
void loguearInstruccion(instruccion* instruccion,int pid, char* resultado);
void loguearContexto(proceso* proceso);
void loguearRafaga(int pid);


#endif /* LOGUEOS_H_ */
