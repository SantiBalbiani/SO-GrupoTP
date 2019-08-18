/*
 * TLB.c
 *
 *  Created on: 1/12/2015
 *      Author: utnso
 */

#include "TLB.h"
#include "logueos.h"
//#include "adm.h"
//#include "configuraciones.h"

t_tlb* createTLB()
{
	t_tlb* tlb = malloc(sizeof(t_tlb));
	tlb->idProceso = -1;
	tlb->nroMarco = -1;
	tlb->nroPagina = -1;
	return tlb;
}

void destroyTLB(t_tlb* tlb)
{
	free(tlb);
}

void limpiarTLB(t_proceso* proceso)
{
	int i;
	waitSemaforo(mutexIdProceso);
	idProceso = proceso->id;
	waitSemaforo(mutexTLB);
	for(i=0; i<list_size(TLB);i++)
	{
		list_remove_and_destroy_by_condition(TLB, (void*)estaProcesoEnTLB, (void*)destroyTLB);
	}
	signalSemaforo(mutexTLB);
	signalSemaforo(mutexIdProceso);
}

bool estaProcesoEnTLB(t_tlb* tlb)
{
	return tlb->idProceso==idProceso;
}

bool estaPaginaEnTLB(t_tlb* tlb)
{
	return ((tlb->idProceso == idProceso) && (tlb->nroMarco==nroMarco) && (tlb->nroPagina== nroPagina));
}

void usarTLB(t_marco* marco)
{
	waitSemaforo(mutexIdProceso);
	waitSemaforo(mutexNroMarco);
	waitSemaforo(mutexNroPagina);			//una paja los semaforos...

	idProceso = marco->idProceso;
	nroMarco = marco->nroMarco;
	nroPagina = marco->pagina->numero;

	waitSemaforo(mutexTLB);
	if (!list_any_satisfy(TLB,(void*)estaPaginaEnTLB)) //si no esta en la TLB...
	{

		agregarOReemplazarEnTLB(marco);
		totalesTLB++;
		dormir();
		loguearAccesoMemoria(idProceso,nroPagina,nroMarco);
	}
	else // estaa en la TLB, entonces no duerme
	{
		loguearHit(nroPagina,nroMarco);
		hitsTLB++;
		totalesTLB++;
		//va el log del ojete
	}
	signalSemaforo(mutexTLB);

	signalSemaforo(mutexIdProceso);
	signalSemaforo(mutexNroMarco);
	signalSemaforo(mutexNroPagina);
}



void consultaTLB(t_marco* marco)
{
	if (string_equals_ignore_case(configAdm->tlb_hab,"si"))
	{
		usarTLB(marco);
	}
	else
	{
		dormir(); //como la TLB no esta habilitada, duerme siempre.
	}
}

bool hayVacio(t_tlb* tlb)
{
	return ((tlb->idProceso==-1) && (tlb->nroMarco == -1) && (tlb->nroPagina==-1));
}



void agregarUnoATLB(t_marco* marco)
{
	t_tlb* tlb = createTLB();
	tlb->idProceso = marco->idProceso;
	tlb->nroMarco = marco->nroMarco;
	tlb->nroPagina = marco->pagina->numero;
	list_add(TLB,tlb);
}

void agregarOReemplazarEnTLB(t_marco* marco)
{
	int numeroPagina = marco->pagina->numero;
	if (list_size(TLB)<configAdm->cant_entradas_tlb)
	{
		loguearMiss(false,0,numeroPagina);
		agregarUnoATLB(marco);
	}
	else
	{
		t_tlb* tlbcesita = list_remove(TLB,0);
		loguearMiss(true,tlbcesita->nroPagina,numeroPagina);
		destroyTLB(tlbcesita);
		agregarUnoATLB(marco);
	}
}

void tasaTLB()
{
	double tasaAciertos = 0;
	for(;;)
	{
		usleep(60*1000000);
		if((totalesTLB) != 0)
		{
			tasaAciertos = (double)hitsTLB/totalesTLB;
		}
		waitSemaforo(mutexLog);
		log_info(logAdm,"Tasa de Aciertos TLB: %.2f",tasaAciertos);
		signalSemaforo(mutexLog);
	}
}
