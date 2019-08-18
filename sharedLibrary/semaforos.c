/*
 * semaforos.c
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#include "semaphore.h"
#include <stdlib.h>

sem_t *crearSemaforo(int cantidadInicial)
{
	sem_t *semaforo=malloc(sizeof(sem_t));
	if(sem_init(semaforo,0,cantidadInicial) == -1 ){
		error("Error en operacion semaforo_create.");
		exit(EXIT_FAILURE);
	}
	return semaforo;
}

sem_t *crearMutex()
{
	return crearSemaforo(1);
}

void destruirSemaforo(sem_t *semaforo){
	sem_destroy(semaforo);
}

void waitSemaforo(sem_t *semaforo_id)
{
	if (sem_wait(semaforo_id) == -1 ){
		error("Error en operacion wait_semaforo.");
		exit(EXIT_FAILURE);
	}
}

void signalSemaforo(sem_t *semaforo_id)
{
	if (sem_post(semaforo_id) == -1 ){
		error("Error en operacion signal_semaforo.");
		exit(EXIT_FAILURE);
	}
}
