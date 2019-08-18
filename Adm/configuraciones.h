/*
 * configuraciones.h
 *
 *  Created on: 27/11/2015
 *      Author: utnso
 */

#ifndef CONFIGURACIONES_H_
#define CONFIGURACIONES_H_
#include <commons/config.h>

#include "adm.h"

typedef struct
{
	int puertoEscucha;
	char *ip_swap;
	int puertoSwap;
	int max_marcos;
	int cant_marcos;
	int tam_marco;
	int cant_entradas_tlb;
	char *tlb_hab;
	int retardo_mem;
	char *algoritmoReemplazo;
}t_config_adm;

t_config_adm* configAdm;
t_config_adm* create_config_adm();

void cargarConfig();
void cargarConfigAdm(t_config* configuracionDma);

void destroy_config_adm(t_config_adm* config);

#endif /* CONFIGURACIONES_H_ */
