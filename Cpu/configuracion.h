/*
 * configuracion.h
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include "cpu.h"

typedef struct
{
	char *ipPlanificador;
	int puertoPlanificador;
	char *ipMemoria;
	int puertoMemoria;
	int cantidadHilos;
	double retardo;
}t_config_cpu;

t_config_cpu* configCPU;

t_config_cpu* create_config_cpu();
void destroy_config_cpu(t_config_cpu* config);

void cargarConfig();
void cargarConfigCPU(t_config* configuracionCPU);

#endif /* CONFIGURACION_H_ */
