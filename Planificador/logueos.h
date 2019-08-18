/*
 * logueos.h
 *
 *  Created on: 30/9/2015
 *      Author: utnso
 */

#ifndef LOGUEOS_H_
#define LOGUEOS_H_
#include "pcb.h"
#include "planificador.h"
#include "semaforos.h"

t_log *logPlanif;
sem_t* mutexLog, *mutexLista;

void loguearTiempos(PCB* pcb);
void loguearEvento(PCB* pcb, char* evento); //creada para reutilizar codigo, es llamada en loguear comienzo, fin y seleccionado
void loguearComienzo(PCB* pcb); //loguea el comienzo de ejecucion del proceso
void loguearFin(PCB* pcb); //loguea la finalizacion de un proceso.
void loguearSeleccionado(PCB* pcb); //loguea el pcb seleccionado para ejecutar

void loguearCola(t_queue* cola, char* nombreCola);//loguea una cola

void loguearLista(t_list* lista, char* nombreLista);
char* logueoLista; //se usa para la funcion que itera la lista
void agregarALogueoLista(PCB* pcb);

void loguearColas(); //tambien loguea la lista de ejecutando
void loguearRetornos(char* retornos);//loguea los retornos de la rafaga
void crearLog();
void destruirLog();

char* agregarCerosMil(int mil);

#endif /* LOGUEOS_H_ */
