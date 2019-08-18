/*
 * TLB.h
 *
 *  Created on: 1/12/2015
 *      Author: utnso
 */

#ifndef TLB_H_
#define TLB_H_
#include <commons/collections/list.h>
#include "configuraciones.h"
#include "marco.h"
#include "estructuras.h"

typedef struct
{
	int nroMarco; //me parece que no hace falta, pero bue..
	int nroPagina;
	int idProceso;
}t_tlb;

int hitsTLB;
int totalesTLB;

void tasaTLB();
void agregarATLB(t_marco* marco);
bool hayVacio(t_tlb* tlb);
t_tlb* createTLB();
void destroyTLB(t_tlb* tlb);
void agregarUnoATLB(t_marco* marco);
void agregarOReemplazarEnTLB(t_marco* marco);
bool estaPaginaEnTLB(t_tlb* tlb);
void usarTLB(t_marco* marco);
void consultaTLB(t_marco* marco);
void limpiarTLB(t_proceso* proceso);
bool estaProcesoEnTLB(t_tlb* tlb);

#endif /* TLB_H_ */
