/*
 * planificador.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "pcb.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "consola.h"
#include "sockets.h"
#include <commons/config.h>
#include <commons/temporal.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/txt.h>
#include "configuracion.h"
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "semaforos.h"
#include "logueos.h"

#define SERIALIZADOR "$$"
#define RECEPCION_PROCESO 0
#define FIFO 0
#define RR 1


/********************************************/
typedef struct{
	int fdCpu;
	int cpuId;
	int disponible;
}t_cliCpu;

t_cliCpu *clienteCpu;
t_cliCpu *cpuPrincipal;
t_list *listaCpus;
int quantum;
bool primeraCPU;

int estaDisponible(t_cliCpu *cliente);
int fdDesconectado(t_cliCpu *cliente);
void deshabilitarCpu(t_cliCpu *cliente);
void mostrarCPU(t_cliCpu* cpu);

/********************************************/
pthread_t pidSelect,hiloConsola;
pthread_t hiloEjecucion,hiloBloqueados;

sem_t *mutexListos,*mutexEjecutando,*mutexBloqueados,*mutexPCBs;
sem_t *mutexIDs;
sem_t *mutexCPUs;
sem_t *mutexPCBbuscado, *mutexCPUbuscada,*mutexIDBuscado;
sem_t *semaforoListos,*semaforoBloqueados,*semaforoCPUdisponible;

int algoritmo;
int socketServidor,socketCPU;
fd_set master; // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
int contadorIDs; //Sirve para asignar IDs a los nuevos procesos
int idBuscado; //para encontrar un PCB con ese id en la lista de PCBs
bool buscarID(PCB* pcb);


t_queue *cListos, *cBloqueados; //c = cola
t_list *lEjecutando; //l = lista
t_list *idDisponibles; //lista con los IDs utilizados por proceso y disponibles nuevamente
t_list *PCBs; //todos los PCBs

int fdBusquedaCPU; //variable que se utiliza para buscarCPU, hay
				   //hay que asignarle un valor antes del llamado a la funcion

/**Devuelve true si el fd de la cpu es igual al fdBusquedaCPU*/
bool buscarCpu(t_cliCpu* cpu);

/**Asigna un ID a un nuevo proceso**/
int asignarID();

void entradaSalida();
void dormir();

/**Retorna una CPU disponible, en caso de que no halla devuelve NULL**/
t_cliCpu* cpuDisponible();

/**Devuelve si una CPU esta conectada**/
bool cpuConectada();

/**Crea un PCB segun la ruta del programa y lo agrega a la lista de PCBs**/
PCB* recibir_mCod(char* ruta);

/*Serializa el pcb y lo manda a la cpu, luego la deshabilita*/
void enviarAEjecutar(PCB* pcb, int fdCpu);

/*Si hay procesos listos envia el primero de la cola a ejecutar*/
void pasarAEjecutar();
void sacarDeEjecutar(PCB* pcb);

void pasarABloqueados(PCB* pcb);

/*Envia un proceso a la cola de listos*/
void pasarAListos(PCB* pcb);

/**ni idea, preguntar a agus**/
void ejecutarNuevoProceso(t_cliCpu* cpuLibre);

void procesarMensajeRecibido(char* mensaje,int fdCpu);

/*Busca una cpu segun el id recibido y la habilita,
 * necesaria al recibir un proceso que termino su rafaga */
t_cliCpu* habilitarCPU(int fdCpu);
void deserializarProceso (char** componentes, int fdCpu); //devuelve la CPU liberada
void asociarId(int id,int fdCpu);

PCB* PCBbuscado; //se usa para remover a un PCB de una lista
bool compararPCB(PCB* pcb); //compara si el pcb recibido y PCBbuscado son iguales
void terminarProceso(PCB* pcb); //remueve un proceso de lEjecutando y PCBs y lo destruye
void devolverID(PCB* pcb); //devuelve el id del proceso finalizado

bool esRutaValida(char* ruta);

void setearQuantum();
void crearHilosColas();
void mainEjecucion();
void mainBloqueados();

void crearSemaforos();
void crearColasYListas();
void finalizarPlanificador();//llamar al finalzar el planificador, destruye estructuras
void destruirColas();
void destruirListas();
void destruirSemaforos();
bool compararInt(int a, int b);
void mostrarSerial(char* string); //agrega el "\n" a un string y lo printea
char* serializarAlgoritmo(int algoritmo);


//*********metricas*********//
void tiempoEntraEjecutar(PCB* pcb);
void tiempoEntraListos(PCB* pcb);
void tiempoEspera(PCB* pcb);
void tiempoRespuesta (PCB* pcb);
void tiempoEjecucion(PCB* pcb);
int hor(char* tiempo);
int min(char* tiempo);
int seg(char* tiempo);
int mil(char* tiempo);
int difMilesimasTiempo(char* actual, char* anterior);
int difSegundosTiempo(char* actual, char* anterior);
void excedenteTiempoEjecucion (PCB* pcb);
void excedenteTiempoEspera (PCB* pcb);
void excedenteTiempoRespuesta (PCB* pcb);

int cantidadInstrucciones(PCB* pcb);

void obtenerPorcentajes(t_cliCpu* cpu);

#endif /* PLANIFICADOR_H_ */
