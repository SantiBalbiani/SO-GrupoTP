/*
 * logueos.h
 *
 *  Created on: 29/11/2015
 *      Author: utnso
 */

#ifndef LOGUEOS_H_
#define LOGUEOS_H_
#include <commons/log.h>
#include "adm.h"
#include "estructuras.h"

t_log *logAdm;
void crearLog();
void loguear(char* logueo);
void loguearInicio(t_instruccion* inst);
void loguearLecturaEscritura(t_instruccion* inst,char* funcion);
void loguearLectura(t_instruccion* inst);
void loguearEscritura(t_instruccion* inst);
void loguearAccesoMemoria(int id, int nroPagina, int marco);
void loguearMiss(bool reemplaza,int paginaSaliente, int paginaEntrante);
void loguearHit(int nroPagina,int nroMarco);
void loguearFin(int id,t_proceso* proc);
char* loguearCola(t_list* cola);
void loguearMemoria();
void loguearAccesoASwapFallo(int pid,t_list* iniciales,t_list* finales);

#endif /* LOGUEOS_H_ */
