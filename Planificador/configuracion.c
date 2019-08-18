/*
 * configuracion.c
 *
 *  Created on: 7/9/2015
 *      Author: utnso
 */
#include "configuracion.h"
#include "logueos.h"

t_config_planificador* create_config_planif()
{
	t_config_planificador *config = malloc(sizeof(t_config_planificador));
	config->algoritmo = string_new();
	return config;
}

void destroy_config_planif(t_config_planificador *config)
{
	free(config->algoritmo);
	free(config);
}

void cargarConfigPlanificador(t_config* configuracionPlanificador) {
	configPlanif = create_config_planif();
	configPlanif->puertoEscucha = config_get_int_value(configuracionPlanificador,
			"PUERTO_ESCUCHA");
	 string_append(&configPlanif->algoritmo,config_get_string_value(configuracionPlanificador,
			 "ALGORITMO_PLANIFICACION"));
	configPlanif->quantum = config_get_int_value(configuracionPlanificador,
			"QUANTUM");
}

void cargarConfig()
{
	t_config *configuracionPlanificador = config_create("planificador.conf");
	cargarConfigPlanificador(configuracionPlanificador);
	log_info(logPlanif,"Configuracion dada de alta");
	config_destroy(configuracionPlanificador);
}



