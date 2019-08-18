/*
 * adm.c
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#include "adm.h"
#include "conexiones.h"
#include "logueos.h"
#include "instruccion.h"
#include "algoritmos.h"

t_pagina *createPagina()
{
	t_pagina *pagina = malloc(sizeof(t_pagina));
	pagina->numero = -1;
	pagina->idProceso = -1;
	pagina->contenido = string_new();
	pagina->uso = false;
	pagina->modificado = false;
	return pagina;
}
void destroyPagina(t_pagina *self)
{
	free(self->contenido);
	self->contenido = string_new();
	free(self);
}

t_proceso *createProceso(int id)
{
	t_proceso *proceso = malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->marcos = list_create();
	proceso->fallosDePag = 0;
	proceso->totPagAcc = 0;
	return proceso;
}

void destroyProceso(t_proceso *self)
{
	list_destroy(self->marcos);
	free(self);
}

int procesarMensajeRecibido(t_contexto* contexto)
{
	int fdCpu= contexto->fd;
	t_instruccion* instruccion = deserealizar(contexto->buff); //crea la inst para un manejo mas facil
	if(string_equals_ignore_case(instruccion->funcion,"iniciar"))
	{
		waitSemaforo(mutexInstruccion);
		char* envio = iniciar(instruccion);
		enviarPorSocket(fdCpu,envio); //ejecuta la instruccion y le manda el resultado a cpu
		dormir();
		destroyInstruccionSWAP(instruccion);
		free(contexto);
		signalSemaforo(mutexInstruccion);
		return 0;
	}
	if(string_equals_ignore_case(instruccion->funcion,"leer"))
	{
		waitSemaforo(mutexInstruccion);
		t_marco* marco = leer(instruccion);
		if (marco!=NULL)
		{
			enviarPorSocket(fdCpu,marco->pagina->contenido); //ejecuta la instruccion y le manda el resultado a cpu
		}
		else
		{
			enviarPorSocket(fdCpu,"-1"); //ejecuta la instruccion y le manda el resultado a cpu
		}
		//consultaTLB(marco);
		destroyInstruccionSWAP(instruccion);
		free(contexto);
		signalSemaforo(mutexInstruccion);
		return 0;
	}
	if(string_equals_ignore_case(instruccion->funcion,"escribir"))
	{
		waitSemaforo(mutexInstruccion);
		t_marco* marco = escribir(instruccion);
		if (marco!=NULL)
		{
			enviarPorSocket(fdCpu,marco->pagina->contenido); //ejecuta la instruccion y le manda el resultado a cpu
		}
		else
		{
			enviarPorSocket(fdCpu,"-1");
		}
		//consultaTLB(marco);
		destroyInstruccionSWAP(instruccion);
		free(contexto);
		signalSemaforo(mutexInstruccion);
		return 0;
	}

	//sino entro en ninguno es finalizar
	waitSemaforo(mutexInstruccion);
	char* envio = finalizar(instruccion);
	dormir();
	enviarPorSocket(fdCpu,envio); //ejecuta la instruccion y le manda el resultado a cpu
	destroyInstruccionSWAP(instruccion);
	free(contexto);
	signalSemaforo(mutexInstruccion);
	return 0;
}


void dormir()
{
	usleep(configAdm->retardo_mem*1000000);
}

char* iniciar(t_instruccion* inst)
{
	enviarPorSocket(fdSWAP,serializar(inst));
 	recv(fdSWAP,bufferSWAP,sizeof(bufferSWAP),0);

	if(atoi(bufferSWAP) != -1)//si swap me dice q esta bien lo creo
	{
		t_proceso* proceso = createProceso(inst->pID);
		list_add(procesos,proceso);
		loguearInicio(inst);
		return "0"; //se creo el proceso
	}
	else
	{
		return "-1";//no se pudo crear proceso
	}
}

/*void crearPaginas(int cantidad, t_proceso* proceso) {
	//creo los elementos de la tabla de paginas
	int nroPagina;
	for (nroPagina = 0; nroPagina < cantidad; nroPagina++)
	{
		t_pagina* pagina = createPagina(nroPagina);
		list_add(proceso->paginas, pagina);
	}
}*/

/*ACLARACION: leer y escribir repiten bastante codigo, voy a tratar de solucionarlo
con punteros cuando ya este implementada la TLB ya que en este momento es complicado*/



t_marco* leer(t_instruccion* inst)
{
	char** parametros = string_split(inst->parametros,SEPARADOR_PARAMETROS);
	int pagInst= atoi(parametros[0]);
	loguearLectura(inst);

	t_proceso* proceso = buscarProceso(inst->pID);
	waitSemaforo(mutexListaMarcos);
	bool vacio = list_is_empty(proceso->marcos);
	signalSemaforo(mutexListaMarcos);
	if (!vacio)//el proceso tiene marcos asignados
	{
		t_marco*  marco = procesoTieneMarcos(inst,proceso,pagInst);  //hace lo necesario para traer el marco con la pagina pedida
		proceso->totPagAcc++;
		return marco;
	}
	else //el proceso NO tiene marcos asignados
	{
		waitSemaforo(mutexListaMarcos);
		bool hayMarcos = list_count_satisfying(marcos,(void*)estaUsado)<configAdm->cant_marcos;
		signalSemaforo(mutexListaMarcos);

		if (hayMarcos) //si hay marcos para asignar
		{
			t_marco* marco = asignarMarco(inst,proceso);  //asigna el primer marco
			proceso->totPagAcc++;
			return marco;
		}
		else
		{
			return NULL; //no tiene ningun marco y no hay disponibles
		}
	}

}

t_marco* escribir(t_instruccion* inst)
{
	char** parametros = string_split(inst->parametros,SEPARADOR_PARAMETROS);
	int pagInst= atoi(parametros[0]);
	loguearEscritura(inst);
	t_proceso* proceso = buscarProceso(inst->pID);
	if (!list_is_empty(proceso->marcos))//el proceso tiene marcos asignados
	{
			t_marco*  marco = procesoTieneMarcos(inst,proceso,pagInst);  //hace lo necesario para traer el marco con la pagina pedida
			reemplazarContenido(marco, parametros[1]);
			proceso->totPagAcc++;
			return marco;
	}
	else //el proceso NO tiene marcos asignados
	{

			if ( list_count_satisfying(marcos,(void*)estaUsado)<configAdm->cant_marcos ) //si hay marcos para asignar
			{
				t_marco* marco = asignarMarco(inst,proceso);  //asigna el primer marco
				reemplazarContenido(marco, parametros[1]);
				proceso->totPagAcc++;
				return marco;
			}
			else
			{
				return NULL; //no tiene ningun marco y no hay disponibles
			}
	}


	/*t_pagina* pagina = buscarPagina(proceso,atoi(parametros[0]));
	t_marco* marco = obtenerMarco(proceso,pagina);
	loguearLecturaEscritura(inst->pID,pagina->numero,"escritura");
	if(marco!=NULL)
	{
		if(marco->modificado) //si se modifico el marco
		{
			//creo que hay que enviar lo que estaba a swap, no estoy seguro
		}
		else
		{
			marco->modificado = true;
		}

		marco->contenido = parametros[1];

		return marco->contenido;
	}
	return "-1"; //no tiene ningun marco y no hay disponibles
	*/
}


/*t_pagina* buscarPagina(t_proceso* proceso,int nroPagina)
{
	waitSemaforo(mutexPaginaBuscada);
	waitSemaforo(mutexIdBuscado);
	idBuscado = proceso->id;
	paginaBuscada = nroPagina;
	t_pagina* buscada = list_find(proceso->paginas,(void*)esLaPagina);
	signalSemaforo(mutexPaginaBuscada);
	signalSemaforo(mutexIdBuscado);
	return buscada;
}*/

bool esLaPagina(t_pagina* pagina)
{
	return pagina->numero==paginaBuscada;
}

t_proceso* buscarProceso(int id)
{
	waitSemaforo(mutexIdBuscado);
	idBuscado = id;
	t_proceso* proceso = list_find(procesos,(void*)esElProceso);
	signalSemaforo(mutexIdBuscado);
	return proceso;
}

bool esElProceso(t_proceso* proceso)
{
	return proceso->id == idBuscado;
}

char* finalizar(t_instruccion* inst)
{
	enviarPorSocket(fdSWAP,serializar(inst)); //le avisa a swap q libere memoria
 	recv(fdSWAP,bufferSWAP,sizeof(bufferSWAP),0);
	t_proceso* proceso = buscarProceso(inst->pID);

	limpiarTLB(proceso);
	limpiarMarcos(proceso);


	loguearFin(inst->pID,proceso);

	waitSemaforo(mutexIdBuscado);
	list_remove_by_condition(procesos,(void*)esElProceso);
	signalSemaforo(mutexIdBuscado);

	return "0";

}

void crearListas() {
	//crearSemaforos();
	TLB = list_create();
	procesos = list_create();
	marcos = list_create();
}

void crearSemaforos()
{
	mutexIdProceso = crearMutex();
	mutexNroPagina = crearMutex();
	mutexPaginaBuscada = crearMutex();
	mutexIdBuscado = crearMutex();
	mutexLog = crearMutex();
	mutexInstruccion = crearMutex();
	mutexListaMarcos = crearMutex();
	mutexNroMarco = crearMutex();
	mutexSenales = crearMutex();
	mutexTLB = crearMutex();
}

void destruirSemaforos()
{
	destruirSemaforo(mutexNroMarco);
	destruirSemaforo(mutexIdProceso);
	destruirSemaforo(mutexListaMarcos);
	destruirSemaforo(mutexNroPagina);
	destruirSemaforo(mutexPaginaBuscada);
	destruirSemaforo(mutexIdBuscado);
	destruirSemaforo(mutexLog);
	destruirSemaforo(mutexInstruccion);
	destruirSemaforo(mutexSenales);
	destruirSemaforo(mutexTLB);
}



int main()
{
	//iniciarVariablesGlobales();
	crearLog();
	cargarConfig();
	crearSemaforos();
	crearListas();
	crearMarcos();
	setearAlgoritmo();
	if (string_equals_ignore_case(configAdm->tlb_hab,"si"))
		{
			pthread_create(&hiloAciertosTLB,NULL,(void*)tasaTLB,NULL);
		}

	inicializarSigAction();
	realizarConexiones();

	//finalizarAdm();

	return 0;
}

void setearAlgoritmo()
{
	char* algoritmo = configAdm->algoritmoReemplazo;
	if (string_equals_ignore_case(algoritmo,"FIFO"))
	{
		actualizarCola = &actualizarColaFIFO_CLOCK;
		algoReemplazo = &reemplazoFIFO_LRU;
		return;
	}
	if (string_equals_ignore_case(algoritmo,"LRU"))
	{
		actualizarCola = &actualizarColaLRU;
		algoReemplazo = &reemplazoFIFO_LRU;
		return;
	}
	if (string_equals_ignore_case(algoritmo,"CM") || string_equals_ignore_case(algoritmo,"Clock-M"))
	{
		actualizarCola = &actualizarColaFIFO_CLOCK;
		algoReemplazo = &reemplazoClock;
		return;
	}
}
