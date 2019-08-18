/*
 * consola.h
 *
 *  Created on: 31/8/2015
 *      Author: utnso
 */

#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "semaforos.h"
#include "planificador.h"

/*COMANDOS*/
void correr(char* ruta);
void ps();
void cpu();
void finalizar(char* parametro);

void imprimirIngreso();

/*FUNCIONES CONSOLAS*/
void iniciarProceso (char* ruta); //esta dentro de correr
int idFuncion(char* funcion);
void mostrarComandos();
void limpiarPantalla();
void aplicarFuncion(int idFuncion, char* parametro);
void levantarConsola();
void iniciarConsola();
void salir();


#endif /* CONSOLA_H_ */
