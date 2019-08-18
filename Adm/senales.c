/*
 * senales.c
 *
 *  Created on: 2/12/2015
 *      Author: utnso
 */

#include "senales.h"

void inicializarSigAction()
{
	sigActionStruct.sa_handler = sigHandler;//puntero a la funcion que maneja la señal.
	sigActionStruct.sa_flags = SA_RESTART;
	//sigemptyset(&sigActionStruct.sa_mask);
	sigfillset(&sigActionStruct.sa_mask);
	registrarSeniales();
}

void tlbFlush()
{
	waitSemaforo(mutexTLB);
	list_clean_and_destroy_elements(TLB,(void*)destroyTLB);
	signalSemaforo(mutexTLB);
}

void memClean()
{
	waitSemaforo(mutexTLB);
	t_list* marcosFiltrados = list_filter(marcos,(void*)estaMarcoOcupado);
	int i;
	for (i=0;i<list_size(marcosFiltrados);i++)
	{
		t_marco* marco = list_get(marcosFiltrados,i);
		if (marco->pagina->modificado)
		{
			mandarVictimaASwap(marco, marco->idProceso);
		}
		limpiarMarco(marco);
	}
	list_clean_and_destroy_elements(TLB,(void*)destroyTLB);
	for (i=0;i<list_size(procesos);i++)
	{
		t_proceso* proceso = list_get(procesos,i);
		list_clean(proceso->marcos);
	}
//	signalSemaforo(mutexListaMarcos);
	signalSemaforo(mutexTLB);
}

void memDump()
{
	loguearMemoria();
}

void sigusr1()
{
	waitSemaforo(mutexLog);
	log_info(logAdm,"Señal Recibida tratamiento SIGUSR1");
	signalSemaforo(mutexLog);
	pthread_create(&pIDfflush,NULL,(void*)tlbFlush,NULL);
	pthread_join(pIDfflush,NULL);
	waitSemaforo(mutexLog);
	log_info(logAdm,"Señal Terminada");
	signalSemaforo(mutexLog);
}

void sigusr2()
{
	waitSemaforo(mutexLog);
	log_info(logAdm,"Señal Recibida tratamiento SIGUSR2");
	signalSemaforo(mutexLog);
	pthread_create(&pIDMemClean,NULL,(void*)memClean,NULL);
	pthread_join(pIDMemClean,NULL);
	waitSemaforo(mutexLog);
	log_info(logAdm,"Señal Terminada");
	signalSemaforo(mutexLog);
}

void sigpoll()
{
	waitSemaforo(mutexLog);
	log_info(logAdm,"Señal Recibida tratamiento SIGPOLL");
	signalSemaforo(mutexLog);

	pid_t pidPOLL=fork();
	if (pidPOLL == -1) perror("Error Fork POLL");
	if(pidPOLL==0)
	{
		memDump();
		waitSemaforo(mutexLog);
		log_info(logAdm,"Señal Terminada");
		signalSemaforo(mutexLog);
		exit(EXIT_SUCCESS);
	}

}

void sigHandler(int signal)
{
	waitSemaforo(mutexSenales);
	waitSemaforo(mutexInstruccion);
	switch(signal)
	{
		case SIGUSR1:
			sigusr1();
			break;
		case SIGUSR2:
			sigusr2();
			break;
		case SIGPOLL:
			sigpoll();
			break;
	}
	signalSemaforo(mutexInstruccion);
	signalSemaforo(mutexSenales);
}

void registrarSeniales()
{
    if (sigaction(SIGUSR1, &sigActionStruct, NULL) == -1) perror("sigaction SIGUSR1");
    if (sigaction(SIGUSR2, &sigActionStruct, NULL) == -1) perror("sigaction SIGUSR2");
    if (sigaction(SIGPOLL, &sigActionStruct, NULL) == -1) perror("sigaction SIGPOLL");
}
