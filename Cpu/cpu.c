

/*
 * cpu.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#include "cpu.h"
#include "configuracion.h"
#include "instrucciones.h"
#include "logueos.h"

void finalizarCPU()
{
	log_destroy(logCPU);
	destroy_config_cpu(configCPU);
}

int clientePlanificador()
{
	return clienteDelServidor(configCPU->ipPlanificador,configCPU->puertoPlanificador);//conexion al Planif
}

int clienteDma()
{
	return clienteDelServidor(configCPU->ipMemoria,configCPU->puertoMemoria);//conexion al Planif
}

hiloCpu* createHiloCpu(int id)
{
	hiloCpu* hilo = malloc(sizeof(hiloCpu));
	hilo->id = id;
	hilo->tiempos = list_create();
	return hilo;
}

proceso* createProceso()
{
	proceso* proc = malloc(sizeof(proceso));
	proc->ruta = string_new();
	proc->instrucciones = list_create();
	proc->retornos = list_create();
	return proc;
}

void destroyProceso(proceso* proceso)
{
	free(proceso->ruta);
	list_destroy_and_destroy_elements(proceso->instrucciones,(void*)destroyInstruccion);
	list_destroy(proceso->retornos);
	free(proceso);
}

void destroyHiloCpu(hiloCpu* hilo)
{
	free(hilo);
}

tiempo* createTiempo()
{
	tiempo* tiem = malloc(sizeof(tiempo));
	tiem->inicio = 0;
	tiem->fin = 0;
	tiem->diferencia = 0;
	return tiem;
}

void tiempoActual(tiempo* tiem)
{
	time_t* tiemIni = malloc(sizeof(time_t));
	tiem->inicio = time(tiemIni);
	tiem->fin = tiem->inicio;
	tiem->diferencia = 0;
	free(tiemIni);
}

tiempo* tiempoInicialALista(hiloCpu* hilo)
{
	tiempo* tiem = createTiempo();
	tiempoActual(tiem);//guarda el arranque del proceso
	list_add(hilo->tiempos,(void*)tiem);
	return tiem;
}

void manejarHilo(hiloCpu* hilo)
{



	for (;;) //recepcion planificador
	{
		recv(hilo->fdPlanif, bufferPlanif, sizeof(bufferPlanif), 0);

		waitSemaforo(mutexBufferPlanif);
		//if (!string_ends_with(bufferPlanif,"CPU"))
		//{
			if (bufferPlanif[0]!='\0') //valida que no se haya caido el plani
			{
				char* recepcion = string_new();
				string_append(&recepcion,bufferPlanif);
				memset(bufferPlanif,'\0',sizeof(bufferPlanif));
				signalSemaforo(mutexBufferPlanif);

				procesarMensajePlanificador(hilo,recepcion);
			}
			else
			{
				memset(bufferPlanif,'\0',sizeof(bufferPlanif));
				signalSemaforo(mutexBufferPlanif);

				log_info(logCPU,"Conexion con el Planificador perdida.");
				exit(0);
			}
		//}
	}
}

void conectarPlanificador(hiloCpu* hilo)
{
	hilo->fdPlanif = clientePlanificador();
	if (hilo->fdPlanif == -1)
	{
		exit(EXIT_FAILURE); //Si no se pudo conectar finaliza la ejecucion -> ver si se cae el hilo o el proceso
	}
}

void conectarDma(hiloCpu* hilo)
{
	hilo->fdDma = clienteDma();
	if (hilo->fdDma == -1)
	{
		exit(EXIT_FAILURE); //Si no se pudo conectar finaliza la ejecucion
	}
	loguearCPUConectada(hilo->id);
}

proceso* deserializarPCB (char** arrSerial)
{
	proceso* proc = createProceso();
	proc->id = atoi(arrSerial[1]);
	string_append(&proc->ruta,arrSerial[2]);
	proc->iPointer = atoi(arrSerial[3]);
	proc->quantum = atoi(arrSerial[4]);
	return proc;
}


char* retornosToString(t_list* retornos)
{
	if(!list_is_empty(retornos))
	{
		int i;
		int size = list_size(retornos);
		char* retorno = string_new();
		for(i=0;i<size;i++)
		{
			char* elemento = string_new();
			string_append(&elemento, (char*)list_get(retornos,i));
			string_append(&retorno, elemento);
			string_append(&retorno,"\n");
		}
		return retorno;
	}
	return "No se ejecuto ninguna rafaga";
}


char* serializarProceso (proceso* proc, int tiempoES)
{
	char* serial = string_new();
	int cantInstrucciones = list_size(proc->instrucciones);
	string_append(&serial, string_itoa(ENVIO_PROCESO));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(proc->id));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(proc->iPointer));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, retornosToString(proc->retornos));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(cantInstrucciones));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(tiempoES));
	return serial;
}

void procesarMensajePlanificador(hiloCpu* hilo, char* mensaje)
{
	char** arrSerial = string_split(mensaje, SERIALIZADOR);
	if (string_starts_with(mensaje,"0"))
	{
		tiempo* tiem = tiempoInicialALista(hilo);// agrega el tiempo de arranque al hilo que va a ejecutar.
		proceso* proc = deserializarPCB(arrSerial);
		loguearContexto(proc);
		obtenerInstrucciones(proc);
		int tiempoES = (*ejecutarInstrucciones)(proc,hilo);
		enviarPorSocket(hilo->fdPlanif,serializarProceso(proc,tiempoES));
		tiempoFinALista(hilo, tiem); //agrega tiempos de salida y destruye los innecesarios
	}
	if (string_starts_with(mensaje,"1"))
	{
		if (atoi(arrSerial[1])==0)
		{
			ejecutarInstrucciones = &ejecutarInstruccionesFIFO;
		}
		else
		{
			ejecutarInstrucciones = &ejecutarInstruccionesRR;
		}
	}
}

void tiempoFinALista(hiloCpu* hilo, tiempo* tiem1)
{
	waitSemaforo(mutexTiem2);

	tiem2 = nuevoTiempo(tiem1);
	list_remove_and_destroy_by_condition(hilo->tiempos,(void*)eliminarTiempoDeLista,(void*)destroyTiempo);
	list_add(hilo->tiempos, (void*)tiem2);

	signalSemaforo(mutexTiem2);

}

bool eliminarTiempoDeLista(tiempo* tiem)
{
	return tiem->inicio == tiem2->inicio;
}

tiempo* nuevoTiempo(tiempo* tiem1)
{
	tiempo* tiem2 = createTiempo();
	time_t* tiempo2 = malloc(sizeof(time_t));
	tiem2->inicio = tiem1->inicio;
	tiem2->fin = time(tiempo2);
	tiem2->diferencia = tiem2->fin - tiem2->inicio;
	free(tiempo2);
	return tiem2;
}

void destroyTiempo(tiempo* tiem)
{
	free(tiem);
}

double calcularPorcentaje(hiloCpu* hilo)
{
	time_t* tiempo1 = malloc(sizeof(time_t));
	tActual = time(tiempo1);
	if(!list_is_empty(hilo->tiempos))
	{
		int i;
		for (i=0; i< list_size(hilo->tiempos);i++)
		{
			list_remove_and_destroy_by_condition(hilo->tiempos, (void*)destruirTiemposViejos, (void*)destroyTiempo);
		}
	}
	free(tiempo1);
	int i;
	int sTotales = 0;
	if(!list_is_empty(hilo->tiempos))
	{
		for(i=0; i<list_size(hilo->tiempos) ;i++)
		{
			tiempo* tiem = list_get(hilo->tiempos,i);

			if (tiem->diferencia == 0)
			{
				sTotales += tActual - tiem->inicio;
			}
			if ( (tiem->inicio >= (tActual-60)) && (tiem->fin > (tActual-60)))
			{
				sTotales += tiem->diferencia;
			}
			if ((tiem->inicio < (tActual-60)) && (tiem->fin > (tActual-60)))
			{
				sTotales = tiem->fin - (tActual-60);
			}
		}
		double porcentaje = (double)sTotales*100/60;
		return porcentaje;
	}

	return 0;

}



bool destruirTiemposViejos(tiempo* tiem)
{
	return tiem->fin < (tActual-60); //
}

char* retornoPorc (double porc, int id)
{
	char* ret = string_new();
	string_append(&ret,"cpu ");
	string_append(&ret,string_itoa(id));
	string_append(&ret,": ");
	string_append(&ret,string_from_format("%.2f",porc));
	string_append(&ret,"% \n");
	return ret;
}

void mainPorcentajes(hiloCpu* hilo)
{
	conectarPlanificador(hilo);
	enviarPorSocket(hilo->fdPlanif,serializarID(hilo->id));

	for(;;)
	{
	recv(hilo->fdPlanif, bufferPlanif, sizeof(bufferPlanif), 0);
	if (string_starts_with(bufferPlanif, "2"))
	{
		int i;
		char* ret = string_new();
		string_append(&ret,"2");
		string_append(&ret,SERIALIZADOR);
//		for (i=configCPU->cantidadHilos; i>=0;i--)

		for (i=0;i<configCPU->cantidadHilos;i++)
		{
			hiloCpu* hiloCPU = list_get(hilosCPU,i);
			double porc = calcularPorcentaje(hiloCPU);
			string_append(&ret,retornoPorc(porc,hiloCPU->id));
		}


		enviarPorSocket(hilo->fdPlanif,ret);


		//list_iterate(hilosCPU,(void*) calcularPorcentaje);??????
		}
	else
	{
		if (string_starts_with(bufferPlanif,"1"))
		{
			char** componentes = string_split(bufferPlanif,SERIALIZADOR);
			if (atoi(componentes[1])==0)
			{
				ejecutarInstrucciones = &ejecutarInstruccionesFIFO;
			}
			else
			{
				ejecutarInstrucciones = &ejecutarInstruccionesRR;
			}
		}
	}
	memset(bufferPlanif,'\0',sizeof(bufferPlanif));

	}

}

int main()
{
	crearLog();
	cargarConfig();
	crearListas();
	crearSemaforos();
	crearHilosCPU();
	//pthread_create(&hiloPorcentajes, NULL,(void*)mainPorcentajes,NULL);

	list_iterate(hilosCPU,(void*)esperarHilo);
	pthread_join(hiloPorcentajes, NULL);

	return 0;
}

void crearHilosCPU()
{
	int i;
	for (i=1;i<=configCPU->cantidadHilos;i++)
	{
		hiloCpu* hiloCPU = createHiloCpu(i); //crea hilo con id 1
		list_add(hilosCPU,(void*)hiloCPU);
		conectarPlanificador(hiloCPU);
		conectarDma(hiloCPU);
		enviarPorSocket(hiloCPU->fdPlanif,serializarID(hiloCPU->id));
		pthread_create(&hiloCPU->hilo, NULL,(void*)manejarHilo,hiloCPU);
	}

	hiloCpu* hiloPrincipal = createHiloCpu(0);
	pthread_create(&hiloPrincipal->hilo, NULL,(void*)mainPorcentajes,hiloPrincipal);
}

char* serializarID(int id)
{
	char* retorno = string_new();
	string_append(&retorno,"1");
	string_append(&retorno,SERIALIZADOR);
	string_append(&retorno,string_itoa(id));
	return retorno;
}

void crearSemaforos()
{
	mutexBufferPlanif = crearMutex();
	mutexBufferDma = crearMutex();
	semComandoCpu = crearSemaforo(0);
	mutex_tActual = crearMutex();
	mutexTiem2 = crearMutex();
}

void destruirSemaforos()
{
	destruirSemaforo(mutexBufferPlanif);
	destruirSemaforo(mutexBufferDma);
	destruirSemaforo(semComandoCpu);
	destruirSemaforo(mutex_tActual);
	destruirSemaforo(mutexTiem2);
}

void esperarHilo(hiloCpu* hiloCPU)
{
	pthread_join(hiloCPU->hilo,NULL);
}

void crearListas()
{
	hilosCPU = list_create();
}

void mostrarSerial(char* string)
{
	char* ss = string_new();
	string_append(&ss, string);
	string_append(&ss,"\n");
	printf("%s", ss);
}
