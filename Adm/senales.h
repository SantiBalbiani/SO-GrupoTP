/*
 * senales.h
 *
 *  Created on: 2/12/2015
 *      Author: utnso
 */

#ifndef SENALES_H_
#define SENALES_H_
#include <signal.h>
#include <pthread.h>
#include <stdlib.h>
#include "logueos.h"
#include "adm.h"
#include "marco.h"
#include "algoritmos.h"

//Señales
pthread_t pIDfflush;
pthread_t pIDMemClean;
struct sigaction sigActionStruct;
sigset_t mask;
sigset_t orig_mask;

void sigHandler(int signal);
void inicializarSigAction();
void registrarSeniales();

void tlbFlush();
void memClean();
void memDump();

#endif /* SENALES_H_ */
