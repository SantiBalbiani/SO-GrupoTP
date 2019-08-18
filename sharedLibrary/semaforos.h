/*
 * semaforos.h
 *
 *  Created on: 29/9/2015
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <semaphore.h>

sem_t *crearSemaforo(int cantidadInicial);
sem_t *crearMutex();
void destruirSemaforo(sem_t *semaforo);

void waitSemaforo(sem_t *semaforo);
void signalSemaforo(sem_t *semaforo);

#endif /* SEMAFOROS_H_ */
