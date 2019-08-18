/*
 * logueos.c
 *
 *  Created on: 1/10/2015
 *      Author: utnso
 */
#include "logueos.h"

void crearLog()
{
	remove("cpu.log");

	mutexLog = crearMutex();
	logCPU = log_create("cpu.log","CPU",false,LOG_LEVEL_INFO);
}

void destruirLog()
{
	destruirSemaforo(mutexLog);
	log_destroy(logCPU);
}

void loguearCPUConectada(int id)
{
	char* logueo = string_new();
	string_append(&logueo,"CPU ");
	string_append(&logueo,string_itoa(id));
	string_append(&logueo," conectada al Administrador de Memoria");

	waitSemaforo(mutexLog);
	log_info(logCPU,logueo);
	signalSemaforo(mutexLog);
}

void loguearContexto(proceso* proceso)
{
	char* logueo = string_new();
	string_append(&logueo,"Contexto de ejecucion recibido: ");
	string_append(&logueo,"mProc ");
	string_append(&logueo,string_itoa(proceso->id));
	string_append(&logueo,". Ruta: ");
	string_append(&logueo,proceso->ruta);
	string_append(&logueo,". Instruction Pointer: ");
	string_append(&logueo,string_itoa(proceso->iPointer));
	string_append(&logueo,". Quantum: ");
	if(proceso->quantum!=0)
	{
		string_append(&logueo,string_itoa(proceso->quantum));
	}
	else
	{
		string_append(&logueo,"N/A");
	}
	string_append(&logueo,".");

	waitSemaforo(mutexLog);
	log_info(logCPU,logueo);
	signalSemaforo(mutexLog);
}

void loguearInstruccion(instruccion* instruccion,int pid, char* resultado)
{
	char* logueo = string_new();

	string_append(&logueo,"mProc ");
	string_append(&logueo,string_itoa(pid));
	string_append(&logueo," ejecuto la instruccion: ");
	string_append(&logueo,instruccionToString(instruccion));
	string_append(&logueo," Resultado: ");
	string_append(&logueo,resultado);

	waitSemaforo(mutexLog);
	log_info(logCPU,logueo);
	signalSemaforo(mutexLog);
}

void loguearRafaga(int pid)
{
	char* logueo = string_new();
	string_append(&logueo,"mProc ");
	string_append(&logueo,string_itoa(pid));
	string_append(&logueo," concluyo la ejecucion de su rafaga.");

	waitSemaforo(mutexLog);
	log_info(logCPU,logueo);
	signalSemaforo(mutexLog);
}
