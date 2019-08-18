/*
 * estructuras.h
 *
 *  Created on: 1/12/2015
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <stdbool.h>

typedef struct
{
	int id;
	t_list* marcos;
	int fallosDePag;
	int totPagAcc;
}t_proceso;

typedef struct
{
	int idProceso;
	int numero;
	char* contenido;
	bool uso;
	bool modificado;
}t_pagina;

typedef struct
{
	int idProceso; //id del proceso que lo utiliza
	int nroMarco;
	t_pagina* pagina;
	bool libre;
					//int pagina;
					//char *contenido; los contenidos los tienen las paginas
}t_marco;



#endif /* ESTRUCTURAS_H_ */
