/*
 * configuraciones.c
 *
 *  Created on: 27/11/2015
 *      Author: utnso
 */

#include "configuraciones.h"
#include "logueos.h"

t_config_adm* create_config_adm()
{
	t_config_adm *config = malloc(sizeof(t_config_adm));
	config->ip_swap = string_new();
	config->tlb_hab = string_new();
	config->algoritmoReemplazo = string_new();
	return config;
}

void cargarConfig()
{
	t_config *configuracionDma = config_create("admin_memoria.conf");
	cargarConfigAdm(configuracionDma);
	log_info(logAdm,"Configuracion dada de alta");
	config_destroy(configuracionDma);
}

void cargarConfigAdm(t_config* configuracionDma)
{
	configAdm = create_config_adm();
	configAdm->puertoEscucha = config_get_int_value(configuracionDma,"PUERTO_ESCUCHA");
	string_append(&(configAdm->ip_swap),config_get_string_value(configuracionDma,"IP_SWAP"));
	configAdm->puertoSwap = config_get_int_value(configuracionDma,"PUERTO_SWAP");
	configAdm->max_marcos = config_get_int_value(configuracionDma,"MAXIMO_MARCOS_POR_PROCESO");
	configAdm->cant_marcos = config_get_int_value(configuracionDma,"CANTIDAD_MARCOS");
	configAdm->tam_marco = config_get_int_value(configuracionDma,"TAMANIO_MARCO");
	configAdm->cant_entradas_tlb = config_get_int_value(configuracionDma,"ENTRADAS_TLB");
	string_append(&configAdm->tlb_hab,config_get_string_value(configuracionDma,"TLB_HABILITADA"));
	configAdm->retardo_mem = config_get_int_value(configuracionDma,"RETARDO_MEMORIA");
	string_append(&configAdm->algoritmoReemplazo,config_get_string_value(configuracionDma,"ALGORITMO_REEMPLAZO"));
}

void destroy_config_adm(t_config_adm *config)
{
	free(config->ip_swap);
	free(config->tlb_hab);
	free(config);
}
