/*
 * marco.c
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#include "marco.h"
#include "conexiones.h"
#include "algoritmos.h"

t_marco *createMarco(int numero)
{
	t_marco *marco = malloc(sizeof(t_marco));
	marco->nroMarco = numero;
	marco->libre = true;
	marco->pagina = createPagina();
	return marco;
}

void destroyMarco(t_marco* marco)
{
	destroyPagina(marco->pagina);
	free(marco);
}

void crearMarcos()
{
	int cantidad = configAdm->cant_marcos;
	int i;
	for (i=0;i<cantidad;i++)
	{
		list_add(marcos,createMarco(i));
	}
}

bool paginaTieneMarco(t_marco* marco)
{
	return marco->pagina->numero==nroPagina;
}

t_marco* buscarMarco(t_list* lMarcos, int pagInst)
{
	waitSemaforo(mutexNroPagina);
	nroPagina = pagInst;
	t_marco* marco = list_find(lMarcos,(void*)paginaTieneMarco);
	signalSemaforo(mutexNroPagina);

	return marco;
}

t_marco* asignarMarco(t_instruccion* inst,t_proceso* proceso)
{
	t_list* listaAuxiliar = crearListaAuxiliar(proceso->marcos);
	dormir();
	totalesTLB++; //porque si estoy asignando un marco, es imposib
	char** parametros = string_split(inst->parametros,SEPARADOR_PARAMETROS);
	enviarPorSocket(fdSWAP,serialLectura(inst->pID, "leer", atoi(parametros[0]))); //pide a swap el contenido de la pagina a leer
 	recv(fdSWAP,bufferSWAP,sizeof(bufferSWAP),0);

	waitSemaforo(mutexListaMarcos);
	t_marco* marco = list_find(marcos,(void*)estaLibre);

	marco->idProceso = proceso->id;
	marco->libre = false;
	marco->pagina->idProceso = inst->pID;
	marco->pagina->numero = atoi(parametros[0]);
	marco->pagina->uso = true;
	proceso->fallosDePag++;
	free(marco->pagina->contenido);
	marco->pagina->contenido = string_new();
	string_append(&marco->pagina->contenido,bufferSWAP);
	list_add(proceso->marcos, marco);

	agregarOReemplazarEnTLB(marco);
	loguearAccesoMemoria(marco->idProceso,marco->pagina->numero,marco->nroMarco);

	memset(bufferSWAP,'\0',sizeof(bufferSWAP));

	list_sort(listaAuxiliar,(void*)ordenarMarcos);
	t_list* auxiliarFinal = crearListaAuxiliar(proceso->marcos);
	list_sort(auxiliarFinal,(void*)ordenarMarcos);
	loguearAccesoASwapFallo(inst->pID,listaAuxiliar,auxiliarFinal);
	list_destroy_and_destroy_elements(listaAuxiliar,(void*)destroyMarco);
	list_destroy_and_destroy_elements(auxiliarFinal,(void*)destroyMarco);
	signalSemaforo(mutexListaMarcos);
	return marco;
}

bool ordenarMarcos(t_marco* marco1,t_marco* marco2)
{
	return marco1->nroMarco<marco2->nroMarco;
}

/*t_marco* obtenerMarco(t_proceso* proceso,t_pagina* pagina)
{
	t_marco* marco =
	//t_marco* marco = buscarMarcoDeLaPagina(proceso->id,pagina->numero);
	if(marco==NULL) //si la pagina no tiene marco
	{
		if(proceso->cantidadMarcos<configAdm->max_marcos)//si el proceso no supera el limite de marcos
		{
			marco = getMarcoDisponible();
			if(marco!=NULL) //si se encontro un marco disponible
			{
				proceso->cantidadMarcos++; //se sube el contador
				vincularMarco(marco,pagina,proceso->id);
			}
		}
		else //si no hay disponible
		{
			if(proceso->cantidadMarcos!=0)
			{
				//aca iria el reemplazo
			}
			else
			{
				return NULL;
			}
		}
	}
	return marco;
}*/

/*void vincularMarco(t_marco* marco,t_pagina* pagina, int id)
{
	marco->idProceso = id;
	marco->pagina = pagina;
	pagina->marco = marco->numero;
}*/

/*t_marco* buscarMarcoDeLaPagina(int id,int numPagina)
{
	waitSemaforo(mutexPaginaBuscada);
	waitSemaforo(mutexIdBuscado);
	paginaBuscada = numPagina;
	idBuscado = id;
	t_marco* marco =  list_find(marcos,(void*) esMarcoDePagina);
	signalSemaforo(mutexPaginaBuscada);
	signalSemaforo(mutexIdBuscado);
	return marco;
}*/

void limpiarMarcos(t_proceso* proceso)
{
	waitSemaforo(mutexListaMarcos);
	int i;
	for (i=0; i<list_size(marcos);i++)
	{
		t_marco* marco = list_get(marcos,i);
		if (marco->idProceso==proceso->id)
		{
			limpiarMarco(marco);
			list_clean(proceso->marcos);
		}

	}
	signalSemaforo(mutexListaMarcos);
}

bool esMarcoDelProceso(t_marco* marco)
{
	return marco->idProceso == idBuscado;
}

void limpiarPagina(t_pagina* pagina)
{
	free(pagina->contenido);
	pagina->contenido=string_new();
	pagina->numero=-1;
	pagina->modificado = false;
	pagina->uso = false;
	pagina->idProceso = -1;
}

void limpiarMarco(t_marco* marco)
{
	marco->idProceso = -1;
	limpiarPagina(marco->pagina);
	marco->libre = true;
}

bool esMarcoDePagina(t_marco* marco)
{
	return marco->pagina->numero == paginaBuscada;
}

t_marco* getMarcoDisponible() //otorga un marco y lo setea en ocupado
{
	t_marco* marco =  list_find(marcos,(void*)estaMarcoDisponible);
	if (marco!=NULL)
	{
		marco->libre = false; //lo deshabilita
		return marco;
	}
	return NULL; //no hay ninguno libre
}

bool estaMarcoDisponible(t_marco* marco)
{
	return marco->libre;
}
bool estaMarcoOcupado(t_marco* marco)
{
	return marco->libre==false;
}

//************sicsic**********//

t_marco* procesoTieneMarcos(t_instruccion* inst,t_proceso* proceso, int pagInst)
{
	waitSemaforo(mutexNroPagina);
	nroPagina = pagInst;

	if (list_any_satisfy(proceso->marcos,(void*)paginaTieneMarco)) //algun marco contiene a la pagina pedida
	{
		signalSemaforo(mutexNroPagina);
		t_marco* marco = buscarMarco(proceso->marcos,pagInst); //busca el marco que la contiene
		marco->pagina->uso = true;
		(*actualizarCola)(proceso->marcos,marco->pagina);
		consultaTLB(marco);
		return marco;
	}
	else //ningun marco contiene a la pagina pedida
	{
		signalSemaforo(mutexNroPagina);
		t_marco* marco = reemplazarOAsignarNuevo(proceso, inst);
		return marco;
	}

	return 0; //nunca va a llegar hasta aca
}

bool tiene0_0(t_marco* marco)
{
	return (marco->pagina->uso == false && marco->pagina->modificado == false);
}

bool tiene0_1(t_marco* marco)
{
	return (marco->pagina->uso == false && marco->pagina->modificado);
}

bool esMarcoDeProceso(t_marco* marco)
{
	return marco->idProceso==idProceso;
}

t_marco* reemplazarOAsignarNuevo(t_proceso* proceso, t_instruccion* inst)
{
	int cantDelProc = list_size(proceso->marcos);
		waitSemaforo(mutexListaMarcos);
	int cantGralMarcosUsados = list_count_satisfying(marcos,(void*)estaUsado);

	if ((cantDelProc<configAdm->max_marcos) && (cantGralMarcosUsados<configAdm->cant_marcos)) //hay lugar
	{
		signalSemaforo(mutexListaMarcos);
		t_marco* marco = asignarMarco(inst,proceso); //asignar nuevo marco
		return marco;
	}
	else //no hay lugar
	{
		signalSemaforo(mutexListaMarcos);
		//HACER EL REEMPLAZO DE PAGINA
		t_marco* marco = reemplazo(proceso->marcos,inst);
		proceso->fallosDePag++;
		return marco;
	}
	return 0; //nunca va a llegar hasta aca
}


bool estaUsado(t_marco* marco)
{
	return marco->libre==false;
}

bool estaLibre(t_marco* marco)
{
	return marco->libre;
}

char* serialLectura(int pId, char* funcion, int pagina)
{
	char* paquete_serializado = string_new();
	string_append(&paquete_serializado,string_itoa(pId));
	string_append(&paquete_serializado,"$$");
	string_append(&paquete_serializado, funcion);
	string_append(&paquete_serializado,"$$" );
	string_append(&paquete_serializado,string_itoa(pagina));
	return paquete_serializado;
}

void reemplazarContenido(t_marco* marco, char* nuevoCont)
{
	marco->pagina->modificado = true;
	free(marco->pagina->contenido);
	marco->pagina->contenido = string_new();
	string_append(&marco->pagina->contenido, nuevoCont);
}

int posMarcoAReemplazar(t_marco* marco)
{
	int i;
	for (i=0; i<list_size(marcos);i++)
	{
		t_marco* marcoPos = list_get(marcos,i);
		if (marcoPos==marco)
		{
			return i;
		}
	}
	return -10;
}

