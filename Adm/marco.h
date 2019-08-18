/*
 * marco.h
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#ifndef MARCO_H_
#define MARCO_H_
#include "adm.h"
#include "estructuras.h"



t_marco *createMarco(int numero);
void destroyMarco(t_marco *self);
t_marco* getMarcoDisponible();
bool estaMarcoDisponible(t_marco* marco);
bool estaMarcoOcupado(t_marco* marco);
bool esMarcoDePagina(t_marco* marco);
t_marco* buscarMarcoDeLaPagina(int id,int numPagina);
t_marco* obtenerMarco(t_proceso* proceso,t_pagina* pagina);
void vincularMarco(t_marco* marco,t_pagina* pagina, int id);
void limpiarMarcos(t_proceso* proceso);
void limpiarMarco(t_marco* marco);
bool esMarcoDelProceso(t_marco* marco);
bool ordenarMarcos(t_marco* marco1,t_marco* marco2);
void crearMarcos();

//************sicsic************//
int nroPagina;
int idProceso;
int nroMarco;

t_marco* buscarMarco(t_list* lMarcos, int pagInst);
bool tiene0_0(t_marco* marco);
bool tiene0_1(t_marco* marco);
bool paginaTieneMarco(t_marco* marco);
t_marco* procesoTieneMarcos(t_instruccion* inst,t_proceso* proceso, int pagInst);
bool esMarcoDeProceso(t_marco* marco);
t_marco* reemplazarOAsignarNuevo(t_proceso* proceso, t_instruccion* inst);
bool estaUsado(t_marco* marco);
bool estaLibre(t_marco* marco);
char* serialLectura(int pId, char* funcion, int pagina);
void reemplazarContenido(t_marco* marco, char* nuevoCont);
int posMarcoAReemplazar(t_marco* marco);
t_marco* asignarMarco(t_instruccion* inst,t_proceso* proceso);

#endif /* MARCO_H_ */
