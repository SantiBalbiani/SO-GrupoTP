/*
 * swap.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */
/*
 * swap.h
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "sockets/sockets.h"
#include "sockets.h"
//#include "sockets.h"
#include "commons/config.h"
#include "commons/collections/list.h"
#include "commons/string.h"
#include "commons/log.h"
#include <unistd.h>
#include "semaforos.h"
#define CANTIDAD_FUNCIONES 4


int pIdDeseado;
const char* funcionesConsola[] = { "iniciar", "leer", "escribir","finalizar"};

pthread_t pIdServerSwap;
pthread_t pIdReceptor;
pthread_t pIdCompactadora;

t_list *listaPIds;
extern char **environ;
char * arrayPaginasSwap;

fd_set master; // conjunto maestro de descriptores de fichero
fd_set read_fds; // conjunto temporal de descriptores de fichero para select()

FILE * swapFile;
int pageSize;
int cantidadPaginasSwap;
char * paginaLeida;
char * paginaEscrita;

int paginaOcupada;
int nroPagLibre;
int mProcPaginaOcupada;
int mProcCantidadPaginasOcupadas;

typedef struct
{
	int pId;
	int numPag;//pagina inicial de escritura
	int cantPag;
	int paginasLeidas;
	int paginasEscritas;//cantidad de paginas q ocupa
}t_swap_mem;//estructura para el manejo de la lista de swap


typedef struct
{
        int puertoEscucha;
        char * nombre_swap;
        int cant_paginas;
        int tam_pagina;
        double retardo_swap;
        double retardo_comp;
}t_config_swap;

t_config_swap* configSwap;
t_log *logSwap;

int socketSwap_fd;

sem_t * mutexSwapFile;
sem_t * mutexArrayPaginasSwap;
sem_t * mutexMProcList;


void iniciarlizarArray();
int verEspaciosContiguosArray(int espacioRequerido);

void dormirInstruccion();
void dormirCompactacion();


t_config_swap* create_config_swap();
void destroy_config_swap(t_config_swap* config);

void crearLog();
void cargarConfig();
void cargarConfigSwap(t_config* configuracionSwap);
void crearServer();
void crearClienteSwap();
void finalizarSwap();
void servidorMultiplexor(int puerto);
void inicializarSwapFile();
void abirSwapFile();
void cerrarSwapFile();
void fseekSwapFile(int numPag,int  paginaInicial);
char * leerPaginaSwap(int numPag,int  paginaInicial);
char * escribirPaginaSwap(int numPag ,int  paginaInicial);
void borrarPaginaSwap(int paginaInicial,int cantidadDePaginas);
void rellenarPaginaSwap(int bytesFaltantes,int numPag, int pagInicial);

void compactadora();
void compactarSwapFile();
void compactar();
int buscarPaginaOcupadaMayorQue(int nroPagLibre);
int buscarPrimerPaginaLibre();
bool numeroPaginamenor(t_swap_mem * mProc, t_swap_mem *otro_mProc);
bool mismoMProcPaginaOcupada(t_swap_mem * mProc);




void crearSemaforos();
void destruirSemaforos();


t_swap_mem *swap_mem_create(int pId, int numPag, int cantPag);
void swap_mem_destroy(t_swap_mem *self);
int mismoPIdSwapDeseado(t_swap_mem *swap_mem);
void interpretar(char* buffer);
int idFuncion(char* funcion);


void iniciar(int pId, int cantPag);//pensar bien la logica
void leer(int pId,int pagALeer);
void escribir(int pId,int pagAEscribir, char* texto);
void finalizar(int pId); //pensar bien la logica
void reservarEspacio(int paginaInicial, int cantPag);
void liberarEspacio(int paginaInicial, int cantPag);

/*
 * 1- quitar de lista
 * 2- combinar datos con arrary para borrar de archivo
 * 3- borrar de array
 * 4- borrar de archivo
 * */


/*
typedef struct
{
	int puertoEscucha;
	char* nombreSWAP;

}t_config_SWAP;

t_config_SWAP* configSWAP;

t_log logSWAP;
int socketServidorSWAP,socketDMA;

*/

#endif /* SWAP_H_ */

