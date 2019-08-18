/*
 * algoritmos.h
 *
 *  Created on: 1/12/2015
 *      Author: utnso
 */

#ifndef ALGORITMOS_H_
#define ALGORITMOS_H_
#include <stdbool.h>
#include "configuraciones.h"
#include "marco.h"
#include "estructuras.h"

/*La idea es manejar las listas para que siempre que haya que reemplazar saquemos el
 * primero, no se si en clock se va a poder hacer
 */

void (*actualizarCola)(t_list*,t_pagina*);
t_marco* (*algoReemplazo)(t_list*);

//estas funciones se usan para el caso que nohay que hacer reemplazo
void actualizarColaLRU(t_list* marcos,t_pagina* pagina); //los bool solo estan para el polimorfismo
void actualizarColaFIFO_CLOCK(t_list* marcos,t_pagina* pagina); //los bool solo estan para el polimorfismo
t_marco* reemplazo(t_list* marcos,t_instruccion* instruccion);
t_marco* reemplazoClock(t_list* marcos);
t_marco* reemplazoFIFO_LRU();
void mandarVictimaASwap(t_marco* marcoVictima, int id);
t_marco* buscar0_0(t_list* marcos);
t_marco* buscar0_1(t_list* marcos);
char* traerPaginaDeSwap(int numeroPaginaNueva,t_instruccion* instruccion);
void limpiarContenidoPagina(t_marco* marcoVictima);
void sacarVictimaDeTLB(t_marco* marco);

//mover a adm
char* serialEscribir(int id,t_pagina* pagina);

#endif /* ALGORITMOS_H_ */
