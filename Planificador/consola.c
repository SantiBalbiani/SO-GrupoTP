/*
 * consola.c
 *
 *  Created on: 31/8/2015
 *      Author: utnso
 */

#include "consola.h"
#include "planificador.h"
#include <commons/collections/list.h>

const int CANTIDAD_FUNCIONES = 6;
const char* funcionesConsola[] = { "correr", "ps", "cpu","finalizar","mc","salir"};
char* descripcionesConsola[] = { "Ejecuta un Programa", "Muestra el estado de los procesos",
								 "Lista las CPUs Actuales","Finaliza el Proceso",
								 "Lista los comandos disponibles",
								 "Cierra el planificador"};
/*------------------COMANDOS------------------*/
void correr(char* ruta)
{
	if (!string_equals_ignore_case(ruta,"\n"))
	{
		iniciarProceso(ruta);
	}
	else
	{
		printf("[ERROR] No se ha ingresado ninguna ruta\n");
	}
}

void iniciarProceso (char* ruta)
{
	char** palabras = string_split(ruta,"\n");//para sacar el \n
	if (esRutaValida(palabras[0]))
	{
		PCB* pcb = recibir_mCod(palabras[0]);
		loguearComienzo(pcb);
		pasarAListos(pcb);
	}
	else
	{
		printf("[ERROR] Ruta incorrecta\n");
	}
}

void ps()
{
	waitSemaforo(mutexPCBs);
	list_sort(PCBs,(void*) compararId);
	list_iterate(PCBs,(void*) mostrarPCB);
	signalSemaforo(mutexPCBs);

	if(list_is_empty(PCBs))
	{
		printf("No hay procesos en el sistema\n");
	}
}

void cpu()
{
	if (!list_is_empty(listaCpus))
	{
		char* envio = string_new();
		string_append(&envio,"2");
		string_append(&envio,SERIALIZADOR);
		string_append(&envio," ");
		enviarPorSocket(cpuPrincipal->fdCpu,envio);
	}
	else
	{
		printf("Ingrese un comando >> ");
	}
}

/*
void mostrarCPU(t_cliCpu* cpu)
{
	//obtenerPorcentajes(cpu);
	printf("Cpu %d -> %d \n",cpu->cpuId,cpu->fdCpu);
}*/

bool buscarID(PCB* pcb)
{
	return pcb->id==idBuscado;
}

void finalizar(char* parametro)
{
	waitSemaforo(mutexIDBuscado);
	idBuscado = atoi(parametro);
	PCB* pcbFin = list_find(PCBs,(void*)buscarID);
	signalSemaforo(mutexIDBuscado);

	if (pcbFin!=NULL)
	{
	pcbFin->finPorConsola = true;
	}
	else
	{
		printf("El mProc ingresado no es valido\n");
	}
}

void mostrarComandos() {
	int contador = 0;
	while (contador < CANTIDAD_FUNCIONES) {
		printf("*------------------------------------------*\n");
		printf("COMANDO 	= %s\n", funcionesConsola[contador]);
		printf("DESCRIPCION = %s\n", descripcionesConsola[contador]);
		printf("*------------------------------------------*\n");
		contador += 1;
	}
}

void salir()
{
	finalizarPlanificador();//libera memoria de las estructuras de plani
	exit(0); //Termina el proceso
}
/*-------------------------------------------*/

// ----------CONSOLA IMPLEMENTACION----------//
void limpiarPantalla() {//esto no funciona
	system("clear");
}

int idFuncion(char* funcion) { //NO SE QUE HACE
	int i;
	for (i = 0;(i < CANTIDAD_FUNCIONES) && (strcmp(funcion, funcionesConsola[i]) != 0); i++);
	return (i <= CANTIDAD_FUNCIONES - 1) ? (i + 1) : -1;
}

void aplicarFuncion(int idFuncion,char *parametro) { //selecciona un case en base al numero que llevaba el contador y aplica la funcion recibe el dir

	enum nomFun {
		CORRER = 1,
		PS,
		CPU,
		FINALIZAR,
		MOSTRAR_COMANDOS,
		SALIR,
		};
	// Lo que hace el enum es convertirme lo que dice en enteros

	switch (idFuncion) {
	case CORRER:
		correr(parametro);
		printf("Ingrese un comando >> ");
		break;
	case PS:
		ps();
		printf("Ingrese un comando >> ");
		break;
	case CPU:
		cpu();
		break;
	case FINALIZAR:
		finalizar(parametro);
		printf("Ingrese un comando >> ");
		break;
	case MOSTRAR_COMANDOS:
		mostrarComandos();
		printf("Ingrese un comando >> ");
		break;
	case SALIR:
		salir();
		break;
	default:
		printf("Comando invalido, ingrese \"mc\" para ver los comandos\n");
		printf("Ingrese un comando >> ");
	}
}

void levantarConsola() {
	char opcion[150];
	int idFunc;
	char *comando = string_new();
	char *parametro = string_new();
	fgets(opcion, 150, stdin);
	//scanf("%s", comando);//controlar desde aca el segundo parametro q figura tanto para correr como par afinalizar

	if(!string_equals_ignore_case(opcion,"\n"))
	{
	char** separado = string_split(opcion," ");

	string_append(&comando,separado[0]);

	if (separado[1] != NULL)
	{
		string_append(&parametro,separado[1]);
	}
	else
	{
		char** separado = string_split(opcion,"'\n'");
		free(comando);
		comando = string_new();
		string_append(&comando,separado[0]);
	}

	idFunc = idFuncion(comando);
	aplicarFuncion(idFunc, parametro);
	}
	else
	{
		printf("No se ha ingresado ningun comando \nIngrese un comando >> ");
	}
	levantarConsola();
}



void iniciarConsola()
{
	mostrarComandos();
	printf("Ingrese un comando >> ");
	levantarConsola();
}

/*void levantarConsola() {
	char opcion[50];
	int idFunc;
	char *comando;
	char *parametro;

	printf("Ingrese un comando >> ");
	fgets(opcion, 50, stdin);
	//scanf("%s", comando);//controlar desde aca el segundo parametro q figura tanto para correr como par afinalizar

	comando = string_new();
	parametro = string_new();

	char** separado = string_split(opcion," ");

	string_append(&comando,separado[0]);
	if (separado[1] != NULL)
	{
	string_append(&parametro,separado[1]);
	idFunc = idFuncion(comando);
	aplicarFuncion(idFunc, parametro);
	levantarConsola();
	}else{
		char** separado = string_split(opcion,"'\n'");
		comando = string_new();
		string_append(&comando,separado[0]);
		idFunc = idFuncion(comando);
		aplicarFuncion(idFunc, parametro);
		levantarConsola();
	}
}*/
