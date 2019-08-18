/*
 * planificador.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */
#include "planificador.h"
#include "pcb.h"

/***********************************************************/

static t_cliCpu *cliCpu_creates(int fdCpu, int cpuId, int disponible);
static void cliCpu_destroy(t_cliCpu *self);
int fdQuitar;
int cpuId = 0;

static t_cliCpu *cliCpu_creates(int fdCpu, int cpuId, int disponible)
{
	t_cliCpu *nuevaCliCpu = malloc(sizeof(t_cliCpu));
    nuevaCliCpu->fdCpu = fdCpu;
    nuevaCliCpu->disponible = disponible; //0 true, 1 false
    nuevaCliCpu->cpuId = cpuId;
	return nuevaCliCpu;
}

static void cliCpu_destroy(t_cliCpu *self)
{
    free(self);
}

int estaDisponible(t_cliCpu *cliente)
{
	return (cliente->disponible == 0);
}

int fdDesconectado(t_cliCpu *cliente)
{
	return (cliente->fdCpu == fdQuitar);
}

t_cliCpu* habilitarCpu(int fdCpu)
{
	waitSemaforo(mutexCPUbuscada);
	fdBusquedaCPU = fdCpu;

	waitSemaforo(mutexCPUs);
	t_cliCpu* cpu = list_find(listaCpus,(void*)buscarCpu);
	signalSemaforo(mutexCPUs);

	signalSemaforo(mutexCPUbuscada);

	if (cpu!=NULL)
	{
		cpu->disponible = 0;
		signalSemaforo(semaforoCPUdisponible);
	}
	return cpu;
}



void deshabilitarCpu(t_cliCpu *cliente)
{
	cliente->disponible = 1;
}

bool buscarCpu(t_cliCpu* cpu)
{
	return cpu->fdCpu == fdBusquedaCPU;
}

bool esRutaValida(char* ruta)//no valida si es una carpeta
{
	FILE* archivo = fopen(ruta, "r");
	if (archivo!=NULL)
	{
		fclose(archivo);
		return true;
	}
	return false;
}

/***********************************************************/

void servidorMultiplexor(int puerto)
{
	int fdmax; // número máximo de descriptores de fichero
	int listener; // descriptor de socket a la escucha
	int newfd; // descriptor de socket de nueva conexión aceptada
	char buf[2000]; // buffer para datos del cliente
	int nbytes;
	int yes=1; // para setsockopt() SO_REUSEADDR, más abajo
	int i;

	FD_ZERO(&master); // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	// obtener socket a la escucha
	listener = crearSocket();
	if (listener == -1)
	{
		exit(1);
	}
	// estado del Fd para bind
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
		exit(1);
	}
	// enlazar
	if (bindearSocket(listener,puerto) == -1)
	{
		exit(1);
	}
	// escuchar
	if (escucharEn(listener) == -1)
	{
		exit(1);
	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	fdmax = listener; // por ahora es éste
// bucle principal
	for(;;)
	{
		memset(buf,'\0',sizeof(buf)); //limpio previamente el buffer para eviar conflictos en primer envio
		read_fds = master; // cópialo
		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
		{
			perror("select");
			exit(1);
		}
		// explorar conexiones existentes en busca de datos que leer
		for(i = 0; i <= fdmax; i++)
		{
			if (FD_ISSET(i, &read_fds))
			{
				// ¡¡tenemos datos!!
				if (i == listener)
				{
					// gestionar nuevas conexiones
					if ((newfd = aceptarEntrantes(listener)) == -1)
					{
						perror("accept");
					}
					else
					{
						FD_SET(newfd, &master); // añadir al conjunto maestro
						cpuId++;
						t_cliCpu* nuevaCpu = cliCpu_creates(newfd,cpuId,0);
						if (primeraCPU)
						{
							enviarPorSocket(newfd,serializarAlgoritmo(algoritmo));
							primeraCPU = false;
						}
						else
						{
							log_info(logPlanif,"CPU conectada");
							signalSemaforo(semaforoCPUdisponible);//salva lo de lacpu principal
						}
						list_add(listaCpus,nuevaCpu);
						if (newfd > fdmax)
						{ // actualizar el máximo
							fdmax = newfd;
						}
					}
				}
				else
				{
				// gestionar datos de un cliente
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0)
					{
						// error o conexión cerrada por el cliente
						if (nbytes == 0)
						{
							// conexión cerrada
							printf("selectserver: socket %d desconectado\n", i);
							log_info(logPlanif,"CPU desconectada");
						}
						else
						{
							perror("recv");
						}

						close(i); // ¡Hasta luego!
						fdQuitar = i;
						list_remove_and_destroy_by_condition(listaCpus,(void*)fdDesconectado,(void*)cliCpu_destroy);
						FD_CLR(i, &master); // eliminar del conjunto maestro
					}
					else
					{
						procesarMensajeRecibido(buf,i);
						memset(buf,'\0',sizeof(buf));
					}
				}
			}
		}
	}
}

char* serializarAlgoritmo(int algoritmo)
{
	char* retorno = string_new();
	string_append(&retorno,"1");
	string_append(&retorno,SERIALIZADOR);
	string_append(&retorno,string_itoa(algoritmo));
	return retorno;
}

int asignarID()
{
	int id;

	waitSemaforo(mutexIDs);
	if (list_is_empty(idDisponibles))
	{
		id = contadorIDs;
		contadorIDs++;
	}
	else
	{
		list_sort(idDisponibles, (void*)compararInt);
		id = (int) list_remove(idDisponibles, 0);
	}
	signalSemaforo(mutexIDs);

	return id;
}

bool compararInt(int a,int b)
{
	if (a<b) //nunca van a ser iguales
	{
		return true;
	}
	return false;
}

bool cpuConectada()
{
	waitSemaforo(mutexCPUs);
	int retorno = list_is_empty(listaCpus);
	signalSemaforo(mutexCPUs);

	return !retorno;
}

t_cliCpu* cpuDisponible() //Si no hay disponibles devuelve NULL
{
	waitSemaforo(mutexCPUs);
	t_cliCpu *clienteDisponible = list_find(listaCpus,(void*)estaDisponible);
	signalSemaforo(mutexCPUs);

	return clienteDisponible;
}

void enviarAEjecutar(PCB* pcb, int fdCpu)
{
	enviarPorSocket(fdCpu,serializarPCB(pcb));
}

PCB* recibir_mCod(char* ruta)
{
	PCB* pcb = createPCB(ruta);

	waitSemaforo(mutexPCBs);
	list_add(PCBs,pcb);
	signalSemaforo(mutexPCBs);

	return pcb;
}

int cantidadInstrucciones(PCB* pcb)
{
	FILE* archivo = fopen(pcb->ruta, "r");

	fseek( archivo, 0L, SEEK_END );
	long tamano = ftell(archivo);//
	fseek( archivo, 0L, SEEK_SET );

	char texto[tamano];
	fread(texto,sizeof(char),tamano,archivo);

	char** instrucciones = string_split(texto,"\n");


	t_list* list = list_create();
	int i;
	for(i=0; instrucciones[i]!=NULL; i++)
	{
		list_add(list, (void*)instrucciones[i]);
	}

	int cant = list_size(list);
	fclose(archivo);
	return cant;
}

void tiempoEntraEjecutar(PCB* pcb) {
	free(pcb->tiempos->entradaAEjecutando);
	pcb->tiempos->entradaAEjecutando = string_new();
	string_append(&pcb->tiempos->entradaAEjecutando,temporal_get_string_time());
}

void pasarAEjecutar()
{
	t_cliCpu* cpu = cpuDisponible();
	waitSemaforo(mutexListos);
	if(cpu!=NULL)
	{
		deshabilitarCpu(cpu);

		PCB* pcb = (PCB*) queue_pop(cListos);
		tiempoEspera(pcb);
		pcb->estado = CORRIENDO;

		waitSemaforo(mutexEjecutando);
		list_add(lEjecutando,(void*)pcb);
		signalSemaforo(mutexEjecutando);
		tiempoEntraEjecutar(pcb);

		if (pcb->finPorConsola == true)
		{
			pcb->iPointer = (cantidadInstrucciones(pcb) - 1); //testear si es con o sin -1
		}

		loguearSeleccionado(pcb);
		loguearColas();
		enviarAEjecutar(pcb,cpu->fdCpu);
	}
	signalSemaforo(mutexListos);
}

int hor(char* tiempo)
{
	char** parametros;
	parametros = string_split(tiempo,":");
	int horas;
	horas = atoi(parametros[0]);
	return horas;
}

int min(char* tiempo)
{
	char** parametros;
	parametros = string_split(tiempo,":");
	int minutos;
	minutos = atoi(parametros[1]);
	return minutos;
}

int seg(char* tiempo)
{
	char** parametros;
	parametros = string_split(tiempo,":");
	int segundos;
	segundos = atoi(parametros[2]);
	return segundos;
}

int mil(char* tiempo)
{
	char** parametros;
	parametros = string_split(tiempo,":");
	int milesimas;
	milesimas = atoi(parametros[3]);
	return milesimas;
}

int difSegundosTiempo(char* actual, char* anterior)
{
	int difH;
	int difM;
	int difS;
	difH = hor(actual) - hor(anterior);
	difM = (min(actual) + 60*difH) - min(anterior);
	difS = (seg(actual) + 60*difM) - seg(anterior);

	return difS;
}

int difMilesimasTiempo(char* actual, char* anterior)
{
	int difMil;
	int difMilNeg;
	difMil = mil(actual) - mil(anterior);

	if (difMil<0)
	{
		difMilNeg = (difMil + 1000);
		return difMilNeg;
	}
	else
	{
		return difMil;
	}
}

void excedenteTiempoEspera (PCB* pcb)
{
	int seg;
	int mil;
	seg = pcb->tiempos->mEspera/1000;
	mil = pcb->tiempos->mEspera - (seg*1000);
	pcb->tiempos->sEspera = pcb->tiempos->sEspera + seg;
	pcb->tiempos->mEspera = mil;
}

void tiempoEspera(PCB* pcb)
{
	pcb->tiempos->sEspera= pcb->tiempos->sEspera + difSegundosTiempo(temporal_get_string_time(),pcb->tiempos->entradaAListos);
	pcb->tiempos->mEspera= pcb->tiempos->mEspera + difMilesimasTiempo(temporal_get_string_time(),pcb->tiempos->entradaAListos);
	if (pcb->tiempos->mEspera > 999)
	{
		excedenteTiempoEspera(pcb);
	}
}

void tiempoEntraListos(PCB* pcb) {
	free(pcb->tiempos->entradaAListos);
	pcb->tiempos->entradaAListos = string_new();
	string_append(&pcb->tiempos->entradaAListos, temporal_get_string_time());
}

void pasarAListos(PCB* pcb)
{
	pcb->estado = LISTO;

	waitSemaforo(mutexListos);
	queue_push(cListos,(void*)pcb);
	signalSemaforo(mutexListos);

	tiempoEntraListos(pcb);
	signalSemaforo(semaforoListos);
}

void excedenteTiempoRespuesta (PCB* pcb)
{
	int seg;
	int mil;
	seg = pcb->tiempos->mRespuesta/1000;
	mil = pcb->tiempos->mRespuesta - (seg*1000);
	pcb->tiempos->sRespuesta = pcb->tiempos->sRespuesta + seg;
	pcb->tiempos->mRespuesta = mil;
}

void tiempoRespuesta (PCB* pcb)
{
	if (pcb->tiempos->mRespuesta == 0 && pcb->tiempos->sRespuesta == 0)
	{
		pcb->tiempos->sRespuesta = difSegundosTiempo(temporal_get_string_time(), pcb->tiempos->llegadaAlSistema);
		pcb->tiempos->mRespuesta = difMilesimasTiempo(temporal_get_string_time(), pcb->tiempos->llegadaAlSistema);
		if (pcb->tiempos->mRespuesta > 999)
			{
				excedenteTiempoRespuesta(pcb);
			}
	}
}

void pasarABloqueados(PCB* pcb)
{
	pcb->estado = BLOQUEADO;

	waitSemaforo(mutexBloqueados);
	queue_push(cBloqueados,(void*)pcb);
	signalSemaforo(mutexBloqueados);
	signalSemaforo(semaforoBloqueados);
}

void crearColasYListas()
{
	PCBs = list_create();
	idDisponibles = list_create();
	cListos = queue_create();
	lEjecutando = list_create(); //lista
	cBloqueados = queue_create();
}

void serverPlanificador()
{
	servidorMultiplexor(configPlanif->puertoEscucha);
}

void finalizarPlanificador()
{
	destruirLog();
	destroy_config_planif(configPlanif);
	destruirColas();
	destruirListas();
	destruirSemaforos();
}

void destruirColas()
{
	queue_destroy(cListos);
	queue_destroy(cBloqueados);
}

void destruirListas()
{
	list_destroy(lEjecutando);
	list_destroy(idDisponibles);
	list_destroy_and_destroy_elements(PCBs,*destroyPCB);
}

void procesarMensajeRecibido(char* mensaje,int fdCpu)
{
	char** componentes = string_split(mensaje,SERIALIZADOR);
	if(string_starts_with(mensaje,"0"))
	{
		deserializarProceso(componentes,fdCpu);
	}
	if(string_starts_with(mensaje,"1"))
	{
		asociarId(atoi(componentes[1]),fdCpu);
	}
	if(string_starts_with(mensaje,"2"))
	{
		printf("%s",componentes[1]);
		//log_info(logPlanif,"ingrese un comando");
		char* buffer = string_from_format("Ingrese un comando >> ");
		txt_write_in_stdout(buffer);
	}
}

void asociarId(int id,int fdCpu)
{
	waitSemaforo(mutexCPUbuscada);
	fdBusquedaCPU = fdCpu;
	t_cliCpu* cpuEncontrada = (t_cliCpu*)list_find(listaCpus,(void*)buscarCpu);
	signalSemaforo(mutexCPUbuscada);

	if (cpuEncontrada!=NULL)
	{
		cpuEncontrada->cpuId = id;
		if (id==0)
		{
			waitSemaforo(mutexCPUbuscada);
			fdBusquedaCPU = cpuEncontrada->fdCpu;
			cpuPrincipal = list_remove_by_condition(listaCpus,(void*)buscarCpu);
			signalSemaforo(mutexCPUbuscada);
		}
	}

}

void deserializarProceso (char** componentes, int fdCpu)
{
	habilitarCpu(fdCpu);

	idBusquedaPCB = atoi(componentes[1]);

	PCB* pcb = list_find(PCBs,(void*)buscarId);
	pcb->iPointer = atoi(componentes[2]);

	loguearRetornos(componentes[3]);

	int cantInstrucciones = atoi (componentes[4]); //cantidad de instrucciones totales del proceso
	int tiempoES = atoi (componentes[5]);

	sacarDeEjecutar(pcb);

	//lo que sigue es medio feo, cadena de ifs, no se me ocurre otra forma por ahora
	if (tiempoES!=0)
	{
		pcb->es = tiempoES;
		pasarABloqueados(pcb);
	}
		else
		{
			if ((pcb->iPointer == cantInstrucciones) || (cantInstrucciones==0))
			{
				terminarProceso(pcb);
			}
			else
			{
				pasarAListos(pcb);
			}
		}
}

void excedenteTiempoEjecucion (PCB* pcb)
{
	int seg;
	int mil;
	seg = pcb->tiempos->mEjecucion/1000;
	mil = pcb->tiempos->mEjecucion - (seg*1000);
	pcb->tiempos->sEjecucion = pcb->tiempos->sEjecucion + seg;
	pcb->tiempos->mEjecucion = mil;
}

void tiempoEjecucion(PCB* pcb)
{
	pcb->tiempos->sEjecucion = pcb->tiempos->sEjecucion + difSegundosTiempo(temporal_get_string_time(),pcb->tiempos->entradaAEjecutando);
	pcb->tiempos->mEjecucion = pcb->tiempos->mEjecucion + difMilesimasTiempo(temporal_get_string_time(),pcb->tiempos->entradaAEjecutando);
	if (pcb->tiempos->mEjecucion > 999)
	{
		excedenteTiempoEjecucion(pcb);
	}
}

void sacarDeEjecutar(PCB* pcb)
{
	waitSemaforo(mutexPCBbuscado);
	PCBbuscado = pcb;

	waitSemaforo(mutexEjecutando);
	list_remove_by_condition(lEjecutando,(void*)compararPCB);
	signalSemaforo(mutexEjecutando);
	tiempoEjecucion(pcb);

	signalSemaforo(mutexPCBbuscado);
}

void terminarProceso(PCB* pcb)
{
	loguearFin(pcb);

	waitSemaforo(mutexPCBbuscado);
	PCBbuscado = pcb;

	waitSemaforo(mutexPCBs);
	list_remove_by_condition(PCBs,(void*)compararPCB);
	signalSemaforo(mutexPCBs);

	signalSemaforo(mutexPCBbuscado);

	devolverID(pcb);
	destroyPCB(pcb);
}

void entradaSalida()
{
	PCB* pcb = (PCB*)queue_peek(cBloqueados);
	tiempoRespuesta(pcb);
	sleep(pcb->es);
	queue_pop(cBloqueados);
	pasarAListos(pcb);
}

void devolverID(PCB* pcb)
{
	waitSemaforo(mutexIDs);
	list_add(idDisponibles,(void*) pcb->id);
	signalSemaforo(mutexIDs);
}


void obtenerPorcentajes(t_cliCpu* cpu)
{
	enviarPorSocket(cpu->fdCpu, "comandoCpu");
}

bool compararPCB(PCB* pcb)
{
	return PCBbuscado == pcb;
}

void calcularPorcentajeCpu(t_cliCpu* cpu) //sarasa
{
	//enviarPorSocket(cpu->fdCpu,"comandoCpu");
}


void crearSemaforos()
{
	mutexListos = crearMutex();
	mutexEjecutando = crearMutex();
	mutexBloqueados = crearMutex();
	mutexPCBs = crearMutex();
	mutexIDs = crearMutex();
	mutexCPUs = crearMutex();
	mutexPCBbuscado = crearMutex();
	mutexIDBuscado = crearMutex();
	mutexCPUbuscada = crearMutex();
	semaforoListos = crearSemaforo(0);
	semaforoBloqueados = crearSemaforo(0);
	semaforoCPUdisponible = crearSemaforo(0);
}

void destruirSemaforos()
{
	destruirSemaforo(mutexListos);
	destruirSemaforo(mutexEjecutando);
	destruirSemaforo(mutexBloqueados);
	destruirSemaforo(mutexPCBs);
	destruirSemaforo(mutexIDs);
	destruirSemaforo(mutexCPUs);
	destruirSemaforo(mutexPCBbuscado);
	destruirSemaforo(mutexIDBuscado);
	destruirSemaforo(mutexCPUbuscada);
	destruirSemaforo(semaforoListos);
	destruirSemaforo(semaforoBloqueados);
	destruirSemaforo(semaforoCPUdisponible);
}

int main()
{
	listaCpus = list_create();
	contadorIDs = 1;
	primeraCPU = true;
	crearLog();
	cargarConfig(); //se deberia validar si pudo o no crear la config
	setearQuantum();
	crearSemaforos();
	crearColasYListas();
	pthread_create(&pidSelect, NULL, (void*)serverPlanificador,NULL);
	crearHilosColas();
	pthread_create(&hiloConsola,NULL,(void*)iniciarConsola, NULL);
	pthread_join(hiloConsola,NULL);
	return 0;
}

void setearQuantum()
{
	if(string_equals_ignore_case(configPlanif->algoritmo,"FIFO"))
	{
		algoritmo = FIFO;
		quantum = 0;
	}
	else
	{
		algoritmo = RR;
		quantum = configPlanif->quantum;
	}
}

void crearHilosColas()
{
	pthread_create(&hiloEjecucion, NULL, (void*)mainEjecucion,NULL);
	pthread_create(&hiloBloqueados,NULL,(void*)mainBloqueados, NULL);
}

void mainEjecucion()
{
	while(true)
	{
		waitSemaforo(semaforoListos);
		waitSemaforo(semaforoCPUdisponible);
		pasarAEjecutar();
	}
}

void mainBloqueados()
{
	while(true)
	{
		waitSemaforo(semaforoBloqueados);
		entradaSalida();
	}
}


void mostrarSerial(char* string)
{
	char* ss = string_new();
	string_append(&ss, string);
	string_append(&ss,"\n");
	printf("%s", ss);
}
