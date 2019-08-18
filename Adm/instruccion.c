/*
 * instruccion.c
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#include "instruccion.h"

t_instruccion* createInstruccionSWAP()
{
	t_instruccion* instruccionParaSWAP = malloc(sizeof(t_instruccion));
	instruccionParaSWAP->pID = 0;
	instruccionParaSWAP->funcion = string_new();
	instruccionParaSWAP->parametros = string_new();
	return instruccionParaSWAP;
}

void destroyInstruccionSWAP(t_instruccion* instruccionParaSWAP){
	free(instruccionParaSWAP->funcion);
	free(instruccionParaSWAP->parametros);
	free(instruccionParaSWAP);
}

char* serializar(t_instruccion* instSwap)
{
	char* paquete_serializado = string_new();
	string_append(&paquete_serializado,string_itoa(instSwap->pID));
	string_append(&paquete_serializado,"$$");
	string_append(&paquete_serializado, instSwap->funcion);
	string_append(&paquete_serializado,"$$" );
	string_append(&paquete_serializado,instSwap->parametros);
	return paquete_serializado;
}

t_instruccion* deserealizar(char * instruccion)
{
	char** paqueteSeparado = string_split(instruccion, SEPARADOR);
	t_instruccion* instruccionParaSWAP = createInstruccionSWAP();
	instruccionParaSWAP->pID = atoi(paqueteSeparado[0]);
	string_append(&instruccionParaSWAP->funcion, paqueteSeparado[1]);
	if(paqueteSeparado[2]!= NULL)
	{
		string_append(&instruccionParaSWAP->parametros, paqueteSeparado[2]);
	}
	return instruccionParaSWAP;
}
