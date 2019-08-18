/*
 * logueos.c
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */

#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "logueos.h"
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>

void loguearEvento(PCB* pcb,char* evento)
{
	char* logueo = string_new();
	string_append(&logueo,evento);
	string_append(&logueo,": ");
	string_append(&logueo,PCBtoString(pcb));
	string_append(&logueo,".");

	if (string_equals_ignore_case(evento,"Finaliza"))
	{
		loguearTiempos(pcb);
	}

	waitSemaforo(mutexLog);
	log_info(logPlanif,logueo);
	signalSemaforo(mutexLog);
}

void loguearComienzo(PCB* pcb)
{
	loguearEvento(pcb,"Comienza");
}

char* agregarCerosMil(int mil)
{
	char* sMil;
	sMil= string_itoa(mil);

	if (strlen(sMil)==1) {
		char* finMil = string_new();
		string_append(&finMil,"00");
		string_append(&finMil,sMil);
		return finMil;
	}
	if (strlen(sMil)==2) {
		char* finMil = string_new();
		string_append(&finMil,"0");
		string_append(&finMil,sMil);
		return finMil;
	}
	return sMil;
}

void loguearTiempos(PCB* pcb) //TESTEAR
{
	char* logueo = string_new();
	string_append(&logueo, "Tiempo de EJECUCION: ");
	string_append(&logueo, string_itoa(pcb->tiempos->sEjecucion));
	string_append(&logueo, ".");
	string_append(&logueo, agregarCerosMil(pcb->tiempos->mEjecucion));
	string_append(&logueo, " segundos, Tiempo de RESPUESTA: ");
	string_append(&logueo, string_itoa(pcb->tiempos->sRespuesta));
	string_append(&logueo, ".");
	string_append(&logueo, agregarCerosMil(pcb->tiempos->mRespuesta));
	string_append(&logueo, " segundos, Tiempo de ESPERA: ");
	string_append(&logueo, string_itoa(pcb->tiempos->sEspera));
	string_append(&logueo, ".");
	string_append(&logueo, agregarCerosMil(pcb->tiempos->mEspera));
	string_append(&logueo, " segundos.");
	log_info(logPlanif,logueo);
}

void loguearFin(PCB* pcb)
{
	loguearEvento(pcb,"Finaliza");
}

void loguearSeleccionado(PCB* pcb)
{
	loguearEvento(pcb,"mProc elegido para ejecutar");
}

void loguearCola(t_queue* cola, char* nombreCola)
{
	char* logueo = string_new();
	t_queue* colaAux = queue_create(); //crea cola auxiliar para regresar a la normal
	string_append(&logueo, nombreCola);
	string_append(&logueo, ": ");

	if(!queue_is_empty(cola)) // se hace una extraccion antes del while porque sino queda un "," al principio colgado
	{
		PCB* pcbTemp = queue_pop(cola);
		string_append(&logueo,PCBtoString(pcbTemp));
		queue_push(colaAux,pcbTemp);
	}
	else
	{
		string_append(&logueo, "Vacia");
	}

	while (!queue_is_empty(cola)) //mientras no este vacia recorrer la cola y agrega el PCB al retorno
	{
		string_append(&logueo,",");
		PCB* pcbTemp = queue_pop(cola);
		string_append(&logueo,PCBtoString(pcbTemp));
		queue_push(colaAux,pcbTemp);
	}

	while(!queue_is_empty(colaAux)) //recupera la cola original
	{
		PCB* pcbTemp = queue_pop(colaAux);
		queue_push(cola,pcbTemp);
	}
	queue_destroy(colaAux); //liberamos la cola auxiliar

	string_append(&logueo, ".");

	waitSemaforo(mutexLog);
	log_info(logPlanif,logueo);
	signalSemaforo(mutexLog);
}

void loguearLista(t_list* lista, char* nombreLista)
{
	waitSemaforo(mutexLista);
	logueoLista = string_new();
	string_append(&logueoLista, nombreLista);
	string_append(&logueoLista, ": ");
	if(!list_is_empty(lista)) // se hace una extraccion antes del while porque sino queda un "," al principio colgado
	{
		//ojo, esto desordena la lista
		PCB* pcb = list_remove(lista,0); //se remueve el primero y se lo loguea para tener un buen formato con la ","
		string_append(&logueoLista,PCBtoString(pcb));
		list_iterate(lista,(void*) agregarALogueoLista);
		list_add(lista,pcb);
	}
	else
	{
		string_append(&logueoLista, "Vacia");
	}
	string_append(&logueoLista, ".");

	waitSemaforo(mutexLog);
	log_info(logPlanif,logueoLista);
	signalSemaforo(mutexLog);
	signalSemaforo(mutexLista);
}

void agregarALogueoLista(PCB* pcb)
{
	string_append(&logueoLista,",");
	string_append(&logueoLista,PCBtoString(pcb));
}

void loguearColas()
{
	loguearCola(cListos,"Listos");
	loguearCola(cBloqueados,"Bloqueados");
	loguearLista(lEjecutando,"Ejecutando");
}

void loguearRetornos(char* retornos)
{
	char** arrRetornos = string_split(retornos, "\n");
	int i;
	for (i=0; arrRetornos[i]!=NULL; i++)
	{
		char* logueo = string_new();
		string_append(&logueo,arrRetornos[i]);

		waitSemaforo(mutexLog);
		log_info(logPlanif,logueo);
		signalSemaforo(mutexLog);
	}
}

void crearLog()
{
	remove("planif.log");

	mutexLog = crearMutex();
	mutexLista = crearMutex();

	logPlanif = log_create("planif.log","Planificador",false,LOG_LEVEL_INFO);
	if (logPlanif==NULL)
	{
		printf("No se pudo inicializar el archivo planif.log");
	}
}

void destruirLog()
{
	log_destroy(logPlanif);
	destruirSemaforo(mutexLista);
	destruirSemaforo(mutexLog);
}


