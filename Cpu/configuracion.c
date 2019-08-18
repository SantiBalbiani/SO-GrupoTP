/*
 * configuracion.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */

#include "configuracion.h"
#include "logueos.h"

t_config_cpu* create_config_cpu()
{
	t_config_cpu *config = malloc(sizeof(t_config_cpu));
	config->ipPlanificador = string_new();
	config->ipMemoria = string_new();
	return config;
}
void destroy_config_cpu(t_config_cpu* config)
{
	free(config->ipPlanificador);
	free(config->ipMemoria);
	free(config);
}

void cargarConfig()
{
	t_config *config = config_create("cpu.conf");
	cargarConfigCPU(config);
	log_info(logCPU,"Configuracion dada de alta");
	config_destroy(config);
}

void cargarConfigCPU(t_config* configuracionCPU) {
	configCPU = create_config_cpu();
	 string_append(&configCPU->ipPlanificador,config_get_string_value(configuracionCPU,
			"IP_PLANIFICADOR"));
	configCPU->puertoPlanificador = config_get_int_value(configuracionCPU,
			"PUERTO_PLANIFICADOR");
	string_append(&configCPU->ipMemoria,config_get_string_value(configuracionCPU,
			"IP_MEMORIA"));
	configCPU->puertoMemoria = config_get_int_value(configuracionCPU,
			"PUERTO_MEMORIA");
	configCPU->cantidadHilos = config_get_int_value(configuracionCPU,
			"CANTIDAD_HILOS");
	configCPU->retardo = config_get_double_value(configuracionCPU, "RETARDO");
}


