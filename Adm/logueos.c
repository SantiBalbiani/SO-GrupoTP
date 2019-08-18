/*
 * logueos.c
 *
 *  Created on: 29/11/2015
 *      Author: utnso
 */

#include "logueos.h"
#include "adm.h"

void crearLog()
{
	remove("Adm.log");
	logAdm = log_create("Adm.log","Adm",false,LOG_LEVEL_INFO);
}

void loguearInicio(t_instruccion* inst)
{
	char* logueo = string_new();
	string_append(&logueo,"Proceso mProc creado, ID: ");
	string_append(&logueo,string_itoa(inst->pID));
	string_append(&logueo," , cantidad de paginas asignadas: ");
	string_append(&logueo,inst->parametros);
	string_append(&logueo,".");
	loguear(logueo);
}

void loguearLecturaEscritura(t_instruccion* inst,char* funcion)
{
	//falta lo de la TLB
	char** parametros = string_split(inst->parametros, SEPARADOR_PARAMETROS);
	char* logueo = string_new();
	string_append(&logueo,"Solicitud de ");
	string_append(&logueo, funcion);
	string_append(&logueo," recibida, PID: ");
	string_append(&logueo,string_itoa(inst->pID));
	string_append(&logueo,", N° de pagina: ");
	string_append(&logueo,parametros[0]);
	string_append(&logueo,".");
	loguear(logueo);
}

void loguearLectura(t_instruccion* inst)
{
	loguearLecturaEscritura(inst,"lectura");
}

void loguearEscritura(t_instruccion* inst)
{
	loguearLecturaEscritura(inst,"escritura");
}

void loguearMiss(bool reemplaza,int paginaSaliente, int paginaEntrante)
{
	char* logueo = string_new();
	string_append(&logueo,"TLB miss");
	if (reemplaza)
	{
		string_append(&logueo,". Resultado del algoritmo de reemplazo: Nº de pagina saliente: ");
		string_append(&logueo,string_itoa(paginaSaliente));
		string_append(&logueo,", , Nº de pagina entrante: ");
		string_append(&logueo,string_itoa(paginaEntrante));
	}
	string_append(&logueo,", se agrego la pagina ");
	string_append(&logueo,string_itoa(paginaEntrante));
	string_append(&logueo,", a la TLB.");
	loguear(logueo);
}

void loguearHit(int nroPagina,int nroMarco)
{
	char* logueo = string_new();
	string_append(&logueo,"TLB hit, Nº de pagina: ");
	string_append(&logueo,string_itoa(nroPagina));
	string_append(&logueo,", Nº de marco: ");
	string_append(&logueo,string_itoa(nroMarco));
	string_append(&logueo,".");
	loguear(logueo);
}


void loguearAccesoASwapFallo(int pid,t_list* iniciales,t_list* finales)
{
	char* logueo = string_new();
	string_append(&logueo,"Acceso a swap (fallo de pagina), resultado del algoritmo"
			" de sustitucion de paginas: \n"
			" 				Estado inicial: ");
	string_append(&logueo,loguearCola(iniciales));
	string_append(&logueo,"\n				Estado final: ");
	string_append(&logueo,loguearCola(finales));
	loguear(logueo);
}

char* loguearCola(t_list* cola)
{
	char* logueoCola = string_new();
	int i;
	if (!list_is_empty(cola))
	{
		for (i=0;i<list_size(cola);i++)
		{
			t_marco* marcoTemp = list_get(cola,i);
			string_append(&logueoCola,"\n				Nº de marco: ");
			string_append(&logueoCola,string_itoa(marcoTemp->nroMarco));
			string_append(&logueoCola,", Nº de pagina: ");
			string_append(&logueoCola,string_itoa(marcoTemp->pagina->numero));
			string_append(&logueoCola,".");
		}
	}
	else
	{
		string_append(&logueoCola,"\n				Cola inicial vacia");
	}
	return logueoCola;
}

void loguearMemoria()
{
	waitSemaforo(mutexListaMarcos);
	t_list* marcosLog = list_filter(marcos,(void*)estaMarcoOcupado);
	char* logueo = string_new();
	string_append(&logueo,"Contenido de la memoria principal:");
	int i;
	if (!list_is_empty(marcosLog))
	{
		for (i=0;i<list_size(marcosLog);i++)
		{
			t_marco* marco = list_get(marcosLog,i);
			string_append(&logueo,"\n					N° de marco: ");
			string_append(&logueo,string_itoa(marco->nroMarco));
			string_append(&logueo,", contenido: ");
			string_append(&logueo,marco->pagina->contenido);
		}
	}
	else
	{
		string_append(&logueo," la memoria principal esta vacia");
	}
	loguear(logueo);
	signalSemaforo(mutexListaMarcos);
}

void loguearAccesoMemoria(int id, int nroPagina, int marco)
{
	char* logueo = string_new();
	string_append(&logueo,"Acceso a memoria realizado, PID: ");
	string_append(&logueo,string_itoa(id));
	string_append(&logueo,",N° de pagina: ");
	string_append(&logueo,string_itoa(nroPagina));
	string_append(&logueo,",N° de marco: ");
	string_append(&logueo,string_itoa(marco));
	string_append(&logueo,".");
	loguear(logueo);
}

void loguearFin(int id,t_proceso* proc)
{
	char* logueo = string_new();
	string_append(&logueo,"Proceso mProc finalizado, ID: ");
	string_append(&logueo,string_itoa(id));
	string_append(&logueo,".\nFallos de pagina: ");
	string_append(&logueo, string_itoa(proc->fallosDePag));
	string_append(&logueo,". Paginas accedidas: ");
	string_append(&logueo,string_itoa(proc->totPagAcc));
	string_append(&logueo,".");

	loguear(logueo);
}

void loguear(char* logueo)
{
	waitSemaforo(mutexLog);
	log_info(logAdm,logueo);
	signalSemaforo(mutexLog);
}
