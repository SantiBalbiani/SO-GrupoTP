/*
 * configuracion.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include "planificador.h"

typedef struct
{
	int puertoEscucha;
	char *algoritmo;
	int quantum;
}t_config_planificador;

t_config_planificador* configPlanif;

t_config_planificador* create_config_planif();
void destroy_config_planif(t_config_planificador* config);

void cargarConfig();
void cargarConfigPlanificador(t_config* configuracionPlanificador);

#endif /* CONFIGURACION_H_ */
