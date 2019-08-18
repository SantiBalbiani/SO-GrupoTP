/*
 * pcb.h
 *
 *  Created on: 15/9/2015
 *      Author: utnso
 */

#ifndef PCB_H_INCLUDED
#define PCB_H_INCLUDED

#include <stdbool.h>
#include <time.h>
#include <commons/string.h>
#include <stdio.h>
#include <stdlib.h>

enum Estado {
	LISTO,
	CORRIENDO,
	BLOQUEADO,
	TERMINADO
	};

typedef struct {
	char* llegadaAlSistema; //será la primer entrada a listos
	char* entradaAListos;
	char* entradaAEjecutando;
	char* entradaABloqueados;
	int mEspera;
	int sEspera;
	int mRespuesta;
	int sRespuesta;
	int mEjecucion;
	int sEjecucion;
}Tiempos;

typedef struct {
	int id;
	char* ruta; //ruta del mcod
	enum Estado estado;
	int iPointer; //instruction pointer
	int es; //tiempo q tiene que hacer es
	Tiempos* tiempos;
	bool finPorConsola;
}PCB;

Tiempos* createTiempos();

/*--Manejo de estructuras--*/
PCB* createPCB();
void destroyPCB();

/*--Funciones--*/
char* serializarPCB (PCB* pcb); //para que CPU pueda deserializarlo y convertirlo a la estructura proceso
void mostrarPCB(PCB* pcb);
char* PCBtoString(PCB*pcb);
char* getNombre(char* ruta);
char* estadoToString(enum Estado estado);
/*Se usa para pasarselo a list_find, busca un PCB a partir
de su id */
bool buscarId(PCB* pcb);
bool PCBterminado (PCB* pcb);
int idBusquedaPCB;
bool compararId(PCB* pcb1,PCB* pcb2);

#endif /* PCB_H_ */

