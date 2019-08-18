/*
 * instrucciones.h
 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */

#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_
#define INICIO 0
#define FALLO 1
#define INSTRUCCION_INCORRECTA 10

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/txt.h>
#include "cpu.h"


enum TipoInstruccion
{
	INICIAR,
	LEER,
	ESCRIBIR,
	ES, //Entrada-salida
	FINALIZAR
};

typedef struct
{
	enum TipoInstruccion tipo;
	int argumento1;
	char* argumento2;
}instruccion;


instruccion* createInstruccion();
void destroyInstruccion(instruccion* inst);

instruccion* string_to_instruccion(char* instruccion);
t_list* string_to_instrucciones(char* contenido);//asd
void obtenerInstrucciones(proceso* proceso);
char* txt_read(FILE* archivo); //testear
enum TipoInstruccion stringToTipo(char* palabra);
char* tipoToString(enum TipoInstruccion tipo);
char* instruccionToString(instruccion* instruccion);
void imprimirInstruccion(instruccion* inst);//
char* ejecutarInstruccion(instruccion* inst, hiloCpu* hilo, int pid,proceso* proceso);
void procesarInstruccion(instruccion* inst,hiloCpu* hilo, int id, proceso* proc);
void terminarRafaga(proceso* proc, int iPointer);
int ejecutarInstruccionesFIFO (proceso* proc, hiloCpu* hilo);
int ejecutarInstruccionesRR (proceso* proc, hiloCpu* hilo);
char* serializarInstruccion(instruccion* inst, int idCpu, int pid);
void dormir();

//Funciones instrucciones (el ID es del mProc que recibe)
//devuelven el "retorno" que sera agregado en la lista de retornos
char* iniciar(instruccion* inst,hiloCpu* hilo, int pid,proceso* proc);
int recibirRespuestaIniciar(int fdDma);

char* leer(instruccion* inst,hiloCpu* hilo, int pid,proceso* proc);
char* escribir(instruccion* inst,hiloCpu* hilo, int pid,proceso* proc);
char* recibirRespuestaLecturaEscritura(int fdDma,proceso* proc);

char* entradaSalida(int pid,int tiempo);
char* finalizar(instruccion* inst,hiloCpu* hilo, int pid);

void sacarComillas(instruccion* inst);

char* crearArgumentoDos(char* linea, char* p0, char* p1);

#endif /* INSTRUCCIONES_H_ */
