/*
 * pcb.c
 *
 *  Created on: 15/9/2015
 *      Author: utnso
 */

#include "pcb.h"
#include "planificador.h"

PCB* createPCB(char* ruta)
{
	PCB* pcb = malloc(sizeof(PCB));
	pcb->id = asignarID();
	pcb->ruta = string_new();
	string_append(&pcb->ruta, ruta);
	pcb->iPointer = 0;
	pcb->es = false;
	pcb->finPorConsola = false;
	pcb->tiempos = createTiempos();
	return pcb;
}

Tiempos* createTiempos()
{
	Tiempos* tiempos = malloc(sizeof(Tiempos));
	tiempos->llegadaAlSistema = string_new();
	string_append(&tiempos->llegadaAlSistema,temporal_get_string_time());
	tiempos->entradaAListos = string_new();
	tiempos->entradaAEjecutando = string_new();
	tiempos->entradaABloqueados = string_new();
	tiempos->mEjecucion = 0;
	tiempos->mEspera = 0;
	tiempos->mRespuesta = 0;
	tiempos->sEjecucion = 0;
	tiempos->sEspera = 0;
	tiempos->sRespuesta = 0;
	return tiempos;
}


void destroyPCB(PCB *pcb)
{
	free(pcb->ruta);
	free(pcb);
}

void mostrarPCB(PCB* pcb)
{
	char* pcbString = PCBtoString(pcb);
	char* estado = estadoToString(pcb->estado);
	printf ("%s -> %s \n",pcbString,estado);
}

char* PCBtoString(PCB*pcb)//usado para mostrarPCB y log
{
	char* retorno = string_new();
	string_append(&retorno,"mProc: ");
	string_append(&retorno,string_itoa(pcb->id));
	string_append(&retorno," ");
	string_append(&retorno,getNombre(pcb->ruta));
	return retorno;
}

char* getNombre(char* ruta)//TESTEAR
{
	char** separados = string_split(ruta,"/"); //ver si funca el /
	int i;
	for (i=0;;i++)
	{
		if (separados[i] == NULL)
		{
			return separados[i-1];
		}
	}
	return NULL;
}

char* estadoToString(enum Estado estado)//TESTEAR
{
	switch(estado)
	{
	case LISTO:
		return "Listo";
		break;
	case CORRIENDO:
		return "Ejecutando";
		break;
	case BLOQUEADO:
		return "Bloqueado";
		break;
	case TERMINADO:
		return "Terminado";
		break;
	default:
		return NULL;
		break;
	}
}

bool buscarId(PCB* pcb)
{
	return pcb->id == idBusquedaPCB;
}

bool compararId(PCB* pcb1,PCB* pcb2)
{
	if (pcb1->id<pcb2->id) //nunca van a ser iguales
	{
		return true;
	}
	return false;
}

bool PCBterminado (PCB* pcb)
{
	return (pcb->estado==TERMINADO);
}

char* serializarPCB (PCB* pcb)
{
	char* serial = string_new();
	string_append(&serial,"0");
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->id));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, pcb->ruta);
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pcb->iPointer));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(quantum));
	return serial;
}
