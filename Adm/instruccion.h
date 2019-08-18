/*
 * instruccion.h
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#ifndef INSTRUCCION_H_
#define INSTRUCCION_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define SEPARADOR "$$"
#define SEPARADOR_PARAMETROS "@@"

typedef struct
{
	int pID;
	char* funcion;
	char* parametros;
}t_instruccion;

t_instruccion* createInstruccionSWAP();
void destroyInstruccionSWAP(t_instruccion* instruccionParaSWAP);

char* serializar(t_instruccion* instruccion);
t_instruccion* deserealizar(char* recepcion);


#endif /* INSTRUCCION_H_ */
