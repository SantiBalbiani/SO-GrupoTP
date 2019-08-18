/*
 * algoritmos.c
 *
 *  Created on: 1/12/2015
 *      Author: utnso
 */
#include "algoritmos.h"
#include "adm.h"
#include "conexiones.h"

void mandarVictimaASwap(t_marco* marcoVictima, int id)
{
	//si esta modificado la manda a swap
	if (marcoVictima->pagina->modificado)
	{
		enviarPorSocket(fdSWAP,
						serialEscribir(id, marcoVictima->pagina));
		recv(fdSWAP, bufferSWAP, sizeof(bufferSWAP), 0);
		memset(bufferSWAP, '\0', sizeof(bufferSWAP));
	}
}

void limpiarContenidoPagina(t_marco* marcoVictima)
{
	free(marcoVictima->pagina->contenido);
	marcoVictima->pagina->contenido = string_new();
}

char* traerPaginaDeSwap(int numeroPaginaNueva,t_instruccion* instruccion)
{
	//traigo la pagina de swap
	enviarPorSocket(fdSWAP,serialLectura(instruccion->pID, "leer", numeroPaginaNueva));
	recv(fdSWAP, bufferSWAP, sizeof(bufferSWAP), 0);

	char* recepcion = string_new();
	string_append(&recepcion, bufferSWAP);
	memset(bufferSWAP, '\0', sizeof(bufferSWAP));
	return recepcion;
}

t_marco* reemplazoFIFO_LRU(t_list* marcos)
{
	t_marco* marcoVictima = list_remove(marcos,0);
	list_add(marcos,marcoVictima);
	return marcoVictima;
}



void sacarVictimaDeTLB(t_marco* marco)
{
	waitSemaforo(mutexIdProceso);
	waitSemaforo(mutexNroMarco);
	waitSemaforo(mutexNroPagina);
	idProceso = marco->idProceso;
	nroMarco = marco->nroMarco;
	nroPagina = marco->pagina->numero;

	//if (list_any_satisfy(TLB, (void*)estaPaginaEnTLB))
	//{
		list_remove_and_destroy_by_condition(TLB,(void*) estaPaginaEnTLB, (void*)destroyTLB);
	//}
	signalSemaforo(mutexIdProceso);
	signalSemaforo(mutexNroMarco);
	signalSemaforo(mutexNroPagina);
}

t_list* crearListaAuxiliar(t_list* marcos)
{
	t_list* auxiliar = list_create();
	int i;
	if(!list_is_empty(marcos))
	{
		for(i = 0; i<list_size(marcos);i++)
		{
			list_add(auxiliar,copiarMarco(list_get(marcos,i)));
		}
	}
	return auxiliar;
}

//solo copia numero de marco y pagina
t_marco* copiarMarco(t_marco* marco)
{
	t_marco* nuevoMarco = createMarco(marco->nroMarco);
	t_pagina* nuevaPagina = createPagina();
	nuevaPagina->numero = marco->pagina->numero;
	nuevoMarco->pagina = nuevaPagina;
	return nuevoMarco;
}


t_marco* reemplazo(t_list* marcos,t_instruccion* instruccion)
{
	waitSemaforo(mutexListaMarcos);
	t_list* listaAuxiliar = crearListaAuxiliar(marcos);
	char** parametros = string_split(instruccion->parametros,SEPARADOR_PARAMETROS);

	dormir();
	totalesTLB++;

	t_marco* marcoVictima = (*algoReemplazo)(marcos);
	mandarVictimaASwap(marcoVictima, instruccion->pID);
	sacarVictimaDeTLB(marcoVictima);

	int numeroPaginaNueva = atoi(parametros[0]);
	char* recepcion = string_new();
	string_append(&recepcion,traerPaginaDeSwap(numeroPaginaNueva,instruccion));

	marcoVictima->pagina->numero = atoi(parametros[0]);
	limpiarContenidoPagina(marcoVictima);

	marcoVictima->pagina->uso = true;
	if (parametros[1]!=NULL) //si es escribir
	{
		marcoVictima->pagina->modificado = true;
		string_append(&marcoVictima->pagina->contenido,parametros[1]);
	}
	else //si es leer
	{
		marcoVictima->pagina->modificado = false;
		free(marcoVictima->pagina->contenido);
		marcoVictima->pagina->contenido = string_new();
		string_append(&marcoVictima->pagina->contenido,recepcion);
	}

	list_sort(listaAuxiliar,(void*)ordenarMarcos);
	t_list* auxiliarFinal = crearListaAuxiliar(marcos);
	list_sort(auxiliarFinal,(void*)ordenarMarcos);
	loguearAccesoASwapFallo(instruccion->pID,listaAuxiliar,auxiliarFinal);
	list_destroy_and_destroy_elements(listaAuxiliar,(void*)destroyMarco);
	list_destroy_and_destroy_elements(auxiliarFinal,(void*)destroyMarco);
	signalSemaforo(mutexListaMarcos);

	agregarOReemplazarEnTLB(marcoVictima);
	loguearAccesoMemoria(marcoVictima->idProceso,marcoVictima->pagina->numero,marcoVictima->nroMarco);

	return marcoVictima;
}

t_marco* reemplazoClock(t_list* marcos)
{
	t_marco* marcoVictima;
	while(true)
	{
		marcoVictima = buscar0_0(marcos);
		if (marcoVictima==NULL)
		{
			marcoVictima = buscar0_1(marcos);
			if(marcoVictima!=NULL)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return marcoVictima;
}

t_marco* buscar0_0(t_list* marcos)
{
	int i;
	for(i=0;i<list_size(marcos);i++)
	{
		t_marco* marco = list_remove(marcos,0);
		if (tiene0_0(marco))
		{
			list_add(marcos,marco);
			return marco;
		}
		list_add(marcos,marco);
	}
	return NULL;
}

t_marco* buscar0_1(t_list* marcos)
{
	int i;
	for(i=0;i<list_size(marcos);i++)
	{
		t_marco* marco = list_remove(marcos,0);
		if (tiene0_1(marco))
		{
			list_add(marcos,marco);
			return marco;
		}
		marco->pagina->uso = false;
		list_add(marcos,marco);
	}
	return NULL;
}

char* serialEscribir(int id,t_pagina* pagina)
{
	char* paquete_serializado = string_new();
	string_append(&paquete_serializado,string_itoa(id));
	string_append(&paquete_serializado,SEPARADOR);
	string_append(&paquete_serializado, "escribir");
	string_append(&paquete_serializado,SEPARADOR);
	string_append(&paquete_serializado,string_itoa(pagina->numero));
	string_append(&paquete_serializado,SEPARADOR_PARAMETROS);
	string_append(&paquete_serializado,pagina->contenido);
	return paquete_serializado;
}

void actualizarColaLRU(t_list* marcos,t_pagina* pagina)
{
	//log_info(logAdm,"entre en actualizacion");
	waitSemaforo(mutexNroPagina);
	nroPagina = pagina->numero;
	//saca el marco de la lista
	t_marco* marcoPagina = list_remove_by_condition(marcos,(void*)paginaTieneMarco);
	signalSemaforo(mutexNroPagina);

	//agrega el marco al final de la lista
	list_add(marcos,marcoPagina);
}

void actualizarColaFIFO_CLOCK(t_list* marcos,t_pagina* pagina)
{
	/*no tiene que hacer nada, ya que los marcos se piden a medida
	que se solicitan*/
}

