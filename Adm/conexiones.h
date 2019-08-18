/*
 * conexiones.h
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "adm.h"

//int fdSWAP; esta declarada dos veces (???
fd_set master; // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
int fdSWAP;
int socketAdm_fd;
char bufferCPU[2000];
char bufferSWAP[2000];

pthread_t hiloServer;
void crearServer();
int crearClienteSwap();
void realizarConexiones();

#endif /* CONEXIONES_H_ */
