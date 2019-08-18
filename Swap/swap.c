/*
 * swap.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */
/*
 * swap.c
 *
 *  Created on: 30/8/2015
 *      Author: utnso
 */

#include "swap.h"
#include <commons/config.h>
#include <commons/string.h>
#include <commons/log.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

// extern int  errno; para debugging

void inicializarArray()
{
	memset(arrayPaginasSwap,'l',cantidadPaginasSwap); //le pongo l para saber q esta libre
}

t_swap_mem *swap_mem_create(int pId, int numPag, int cantPag)
{
	t_swap_mem *nSwap_mem = malloc(sizeof(t_swap_mem));
	nSwap_mem->pId = pId;
	nSwap_mem->numPag = numPag;
	nSwap_mem->cantPag = cantPag;
	nSwap_mem->paginasLeidas = 0;
	nSwap_mem->paginasEscritas = 0;
	return nSwap_mem;
}

void swap_mem_destroy(t_swap_mem *self)
{
    free(self);
}

int mismoPIdSwapDeseado(t_swap_mem *swap_mem)
{
	return (swap_mem->pId == pIdDeseado);
}

int idFuncion(char* funcion)
{
	int i;
	for (i = 0;(i <  CANTIDAD_FUNCIONES) && (strcmp(funcion, funcionesConsola[i]) != 0); i++);
	if (i == CANTIDAD_FUNCIONES)  i--;
	return i;
}

int verEspaciosContiguosArray(int paginasRequeridas)
{
	if(cantidadPaginasSwap<paginasRequeridas)
	{
		return -1;//no hay espacio total suficiente
	}
	int i;
	int paginasContiguas = 0;
	int espacioLibre = 0;
	int paginaInicial = 0;
	for(i=0;paginasContiguas < paginasRequeridas && i<cantidadPaginasSwap;i++)
	{
		if(arrayPaginasSwap[i] == 'l')
		{
			paginasContiguas++;
			espacioLibre++;
		}
		else
		{
			paginasContiguas=0;
			paginaInicial = i+1;
		}
	}
	if (espacioLibre<paginasRequeridas)
	{
		return -1;
	}
	if (paginasContiguas>=paginasRequeridas)
	{
		return paginaInicial; //ok, hay espacio libre suficiente contiguo
	}
	if(espacioLibre>=paginasRequeridas)
	{
		return -2;
	}
	return 10;
}

/*int verEspaciosContiguosArray(int espacioRequerido)
{
	int posInicioLibres = 0;
	int contEspacios = 0;
	int i = 0;
	while(contEspacios < espacioRequerido)
	{
		if(arrayPaginasSwap[i] == 'l')
		{
			contEspacios++;
		}
		else
		{
			contEspacios = 0;
			posInicioLibres = i+1;
		}
		//printf("Pos inicial de libres: %d,Contenido: %c, Posicion: %d, Espacios: %d\n",posInicioLibres,arrayPaginasSwap[i],i,contEspacios);
		i++;
		if( (cantidadPaginasSwap  < espacioRequerido) || (posInicioLibres > cantidadPaginasSwap))
		{
			//printf("No existe espacio suficiente\n");
			return -1;//combinamos aca para ver los espacios tmb
		}
	}
	return posInicioLibres;//retorno la pagina inicial para ser escrita
}*/

void iniciar(int pId, int cantPag)
{
	//waitSemaforo(mutexArrayPaginasSwap);
	int  paginaInicial = verEspaciosContiguosArray(cantPag); //retorna la primer pagina a reservar si hay espacio suficiente, si no lo hay retorna -1
	if(paginaInicial>=0)
	{
		reservarEspacio(paginaInicial,cantPag);
		t_swap_mem *pIdSwapMem = swap_mem_create(pId,paginaInicial,cantPag);//0 solo es porque antes se pide la referencia
		waitSemaforo(mutexMProcList);
		list_add(listaPIds,pIdSwapMem);
		signalSemaforo(mutexMProcList);
		//signalSemaforo(mutexArrayPaginasSwap);
		int byteInicial = pageSize * paginaInicial;
		int tamanio = cantPag * pageSize;

		log_info(logSwap,"mProc asignado, PID: %d,Byte Inicial: %d, Tamaño De Bytes: %d",pId,byteInicial,tamanio);
		enviarPorSocket(socketSwap_fd,"0");//se pudo iniciar
	}
	//else
	//{
	//	log_info(logSwap,"mProc finalizado, PID: %d, por falta de espacio",pId,paginaInicial);
	//	enviarPorSocket(socketSwap_fd,"-1");//envio para que maten el proceso por falta de memoria
	//	signalSemaforo(mutexArrayPaginasSwap);
	//}
	if(paginaInicial == -1)
	{
		log_info(logSwap, "mProc %d rechazado por falta de espacio",pId);
		enviarPorSocket(socketSwap_fd,"-1");//envio para que maten el proceso por falta de memoria

	}
	if(paginaInicial == -2)
	{
		compactarSwapFile(); //Si no hay espacio Compractará
		iniciar(pId,cantPag);
	}
}

void reservarEspacio(int paginaInicial,int cantPag){
	int i;
	int tope = paginaInicial + cantPag -1;
	for(i=paginaInicial;i <= tope;i++)
	{
		arrayPaginasSwap[i]='r';
	}
}

void liberarEspacio(int paginaInicial,int cantPag)
{
	int i;
	int tope = paginaInicial + cantPag -1;
	for(i=paginaInicial;i <= tope;i++)
	{
		arrayPaginasSwap[i]='l';
	}
}

void leer(int pId, int pagALeer)
{
	waitSemaforo(mutexArrayPaginasSwap);
	//paginaLeida = string_new();
	memset(paginaLeida,'\0',pageSize);
	pIdDeseado= pId;
	//waitSemaforo(mutexMProcList);
	t_swap_mem *pidSwapMem = list_find(listaPIds,(void*)mismoPIdSwapDeseado);

	if(arrayPaginasSwap[pagALeer+pidSwapMem->numPag] == 'o') //verifico en el arrary si la pagina esta usada en el archivo swap
	{
		//pIdDeseado= pId;
		waitSemaforo(mutexMProcList);
		//t_swap_mem *pidSwapMem = list_find(listaPIds,(void*)mismoPIdSwapDeseado);
		pidSwapMem->paginasLeidas++;
		signalSemaforo(mutexMProcList);

		leerPaginaSwap(pagALeer,pidSwapMem->numPag);
		signalSemaforo(mutexArrayPaginasSwap);

		paginaLeida[pageSize]='\0';//agregaba caracteres basura. con esto corrije ese error

		int byteInicial= pageSize * (pagALeer + pidSwapMem->numPag);
		log_info(logSwap,"Lectura Solicitada, PID: %d,Byte Inicial: %d,Tamaño: %d,Contenido: %s",pId,byteInicial,pageSize,paginaLeida);

/*		char *resultadoDeLectura = string_new();
		string_append(&resultadoDeLectura,string_itoa(pId));
		string_append(&resultadoDeLectura,"$$");
		string_append(&resultadoDeLectura,paginaLeida);*/
		enviarPorSocket(socketSwap_fd,paginaLeida);
	}
	else
	{
		dormirInstruccion();
		pIdDeseado= pId;
		t_swap_mem *pidSwapMem = list_find(listaPIds,(void*)mismoPIdSwapDeseado);
		pidSwapMem->paginasLeidas++;
		int byteInicial= pageSize * (pagALeer + pidSwapMem->numPag);
		log_info(logSwap,"Lectura Solicitada, PID: %d,Byte Inicial: %d,Tamaño: %d,Contenido: %s",pId,byteInicial,pageSize,"Pagina vacia");
		enviarPorSocket(socketSwap_fd,"Pagina vacia");
		signalSemaforo(mutexArrayPaginasSwap);
	}
}

void escribir(int pId,int pagAEscribir, char * texto)
{
	memset(paginaEscrita,'\0',pageSize);//limpio el buff por las dudas =)
	string_append(&paginaEscrita,texto);//cargo el texto en el buffer

	pIdDeseado= pId;
	waitSemaforo(mutexMProcList);
	t_swap_mem *pidSwapMem = list_find(listaPIds,(void*)mismoPIdSwapDeseado);
    pidSwapMem->paginasEscritas++;
	signalSemaforo(mutexMProcList);

	escribirPaginaSwap(pagAEscribir, pidSwapMem->numPag);

	//string_append(&paginaEscrita,"\n"); se quita porque el paquete es mas grande
	waitSemaforo(mutexArrayPaginasSwap);
	arrayPaginasSwap[pagAEscribir+pidSwapMem->numPag]= 'o';
	signalSemaforo(mutexArrayPaginasSwap);

	int byteInicial= pageSize * (pagAEscribir + pidSwapMem->numPag);
	log_info(logSwap,"Escritura Solicitada, PID: %d,Byte Inicial: %d,Tamaño: %d,Contenido: %s",pId,byteInicial,pageSize,paginaEscrita);
	enviarPorSocket(socketSwap_fd,"0");
}

void finalizar(int pId)
{
	waitSemaforo(mutexMProcList);
	t_swap_mem *pidSwapMem = list_find(listaPIds,(void*)mismoPIdSwapDeseado);

	int byteInicial= pageSize *  pidSwapMem->numPag;
	int tamanio = pageSize * pidSwapMem->cantPag;
	int paginaInicial = pidSwapMem->numPag;
	int cantidadDePaginas =pidSwapMem->cantPag;
    int paginasLeidas= pidSwapMem->paginasLeidas;
    int paginasEscritas= pidSwapMem->paginasEscritas;

	list_remove_and_destroy_by_condition(listaPIds,(void*)mismoPIdSwapDeseado,(void*)swap_mem_destroy);
	signalSemaforo(mutexMProcList);

	//funcion que lo saca del archivo tambien tiene que sacar del array
	/*printf("**************\n");
	printf("%d\n",pidSwapMem->pId);
	printf("%d\n",pidSwapMem->numPag);
	printf("%d\n",pidSwapMem->cantPag);Solo a mododo de control =)*/

	//quitar del array

	waitSemaforo(mutexArrayPaginasSwap);
	liberarEspacio(paginaInicial,cantidadDePaginas);
	signalSemaforo(mutexArrayPaginasSwap);

	//librerar del archivo

	borrarPaginaSwap(paginaInicial,cantidadDePaginas);

	enviarPorSocket(socketSwap_fd,"0");

	log_info(logSwap,"mProc liberado, PID: %d,Byte Inicial: %d, Tamaño De Bytes: %d Paginas Leidas: %d Paginas Escritas: %d ",pId,byteInicial,tamanio,paginasLeidas,paginasEscritas);
}

void interpretar(char* buffer) //mejorar para mas funciones
{
	int pIdBuff;
	int funcionId;
	int parametro;//puede ser null

	char** componentes = string_split(buffer,"$$");//para que funcione siempre tiene que tener el simbolo elegido
	pIdBuff = atoi(componentes[0]);

	//printf("\npidbuff: %d componente1: %s\n",pIdBuff,componentes[1]);
	char ** componentesParametro;
	char* texto;
	int pagAEscribir;

	funcionId = idFuncion(componentes[1]);

	switch(funcionId)
	{
		case 0:
			parametro = atoi(componentes[2]);
			iniciar(pIdBuff,parametro);
			break;
		case 1:
			parametro = atoi(componentes[2]);
			leer(pIdBuff,parametro);
			break;
		case 2:
			componentesParametro = string_split(componentes[2],"@@"); //supongo seoarador @@ corregir ante la duda
			texto = string_new();
			pagAEscribir = atoi(componentesParametro[0]);
			string_append(&texto,componentesParametro[1]);

			escribir(pIdBuff,pagAEscribir,texto);

			break;
		case 3:

			pIdDeseado=pIdBuff;
			finalizar(pIdDeseado);
			break;
		default:
			break;

	}
	pthread_exit(&pIdReceptor);
}

void crearLog()
{
	remove("Swap.log");
	logSwap = log_create("Swap.log","Swap",false,LOG_LEVEL_INFO);
}

t_config_swap * create_config_swap()
{
	t_config_swap *config = malloc(sizeof(t_config_swap));
	config->nombre_swap = string_new();
	return config;
}

void cargarConfigSwap(t_config* configuracionSwap)
{
	configSwap = create_config_swap();
	configSwap->puertoEscucha = config_get_int_value(configuracionSwap,"PUERTO_ESCUCHA");
	string_append(&(configSwap->nombre_swap),config_get_string_value(configuracionSwap,"NOMBRE_SWAP"));
	configSwap->cant_paginas = config_get_int_value(configuracionSwap,"CANTIDAD_PAGINAS");
	configSwap->tam_pagina = config_get_int_value(configuracionSwap,"TAMANIO_PAGINA");
	configSwap->retardo_swap = config_get_double_value(configuracionSwap,"RETARDO_SWAP");
	configSwap->retardo_comp = config_get_double_value(configuracionSwap,"RETARDO_COMPACTACION");
}

void cargarConfig()
{
    t_config *configuracionSwap = config_create("admin_swap.conf");
    cargarConfigSwap(configuracionSwap);
    log_info(logSwap,"Configuracion dada de alta");
    config_destroy(configuracionSwap);
}

void servidorMultiplexor(int puerto)
{
	int fdmax; // número máximo de descriptores de fichero
	int listener; // descriptor de socket a la escucha
	int newfd; // descriptor de socket de nueva conexión aceptada
	char buf[256]; // buffer para datos del cliente
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
			{ // ¡¡tenemos datos!!
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
						//agregar(&ptrCabeza,newfd,0);
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
						}
						else
						{
							perror("recv");
						}
						close(i); // ¡Hasta luego!
						//retirar(&ptrCabeza,&ptrTalon,i);
						FD_CLR(i, &master); // eliminar del conjunto maestro
					}
					else
					{
							// tenemos datos de algún cliente
						// ¡enviar a todo el mundo!
						if (FD_ISSET(i, &master))
						{
							/*if (send(i, "ok\n", 3, 0) == -1)//Se coloca el mensaje deseado para
							{								//La conexion q solicito algo
								perror("send");				//Aca estaria la logia adicional
							}*/
							socketSwap_fd = i;
							pthread_create(&pIdReceptor,NULL,(void*)interpretar,buf);
							pthread_join(pIdReceptor,NULL);//creo y espero la interpretacion
							//case de control de la recepcion
							//cada recepcion tiene una respuestas acorde.
							//hay que poner un semaforo por aca...
							memset(buf,'\0',sizeof(buf));
						}
					}
				}
			}
		}
	}
}

void crearServer()
{
    servidorMultiplexor(configSwap->puertoEscucha);
}

void destroy_config_swap(t_config_swap *config)
{
    free(config);
}

void finalizarSwap()
{
	destroy_config_swap(configSwap);
}

void inicializarSwapFile()
{
    char* ddArg = string_new();

    //string_append(&ddArg,"dd if=/dev/urandom of="); //urandom genera caracteres para debug
	string_append(&ddArg,"dd if=/dev/zero of=");
	string_append(&ddArg,configSwap->nombre_swap);
	string_append(&ddArg," count=");
	string_append(&ddArg,string_itoa(configSwap->cant_paginas));
	string_append(&ddArg," bs=");
	string_append(&ddArg,string_itoa(configSwap->tam_pagina));

	log_info(logSwap,"Ejecutando %s para inicializar Archivo Swap",ddArg);
	if ((system(ddArg)) ==-1)
	{
		log_error(logSwap,"Fallo la inicializacion de Archivo de Swap");
	}
	else
	{
		log_info(logSwap,"El Archivo de Swap se inicializo exitosamente");
	}
}

void abirSwapFile()
{
	swapFile = fopen(configSwap->nombre_swap, "r+");
	//cambiar los printf por log de la catedra
	if (swapFile!=NULL)
	{
		log_info(logSwap,"El swapFile: %s  se abrio exitosamente.\n", configSwap->nombre_swap);
		//printf("Archivo swapFile: %s abierto exitosamente.\n", configSwap->nombre_swap);
	}
	else
	{
		log_error(logSwap,"Hubo un error al intentar abrir el swapFile: %s.\n",configSwap->nombre_swap);
		//printf("Fallo apertura del archivo swapFile: %s\n",configSwap->nombre_swap);
	}
}

void cerrarSwapFile()
{
	fclose(swapFile);
}

void fseekSwapFile(int numPag,int  paginaInicial)
{
	int posicionPagina= pageSize * (numPag+  paginaInicial);
	fseek(swapFile,posicionPagina,SEEK_SET);
}

char * leerPaginaSwap(int numPag, int  paginaInicial)
{
	dormirInstruccion();
	waitSemaforo(mutexSwapFile);

	fseekSwapFile(numPag, paginaInicial);
    fread(paginaLeida, sizeof(char), pageSize, swapFile);

    signalSemaforo(mutexSwapFile);

    return paginaLeida;
}

char * escribirPaginaSwap(int numPag, int paginaInicial)
{
	dormirInstruccion();
	waitSemaforo(mutexSwapFile);
	fseekSwapFile(numPag, paginaInicial);
	fwrite(paginaEscrita, sizeof(char), pageSize, swapFile);

	if(string_length(paginaEscrita)<pageSize)
	{ // Se rellena con '\0's si lo que se escribe es menor que pageSize
        int	nullBytesFaltantes = pageSize - string_length(paginaEscrita);
        rellenarPaginaSwap(nullBytesFaltantes,numPag, paginaInicial);
    }
    signalSemaforo(mutexSwapFile);

    return paginaEscrita;
}

void dormirInstruccion()
{
	usleep(configSwap->retardo_swap*1000000);
}

void dormirCompactacion()
{
	usleep(configSwap->retardo_comp*1000000);
}

void borrarPaginaSwap(int paginaInicial,int cantidadDePaginas)
{
	//sleep(configSwap->retardo_swap); -> supuestamente no va
	int mProcSize = pageSize  * cantidadDePaginas;
	char * paginaABorrar = malloc(mProcSize);
	memset(paginaABorrar,'\0',mProcSize);

	waitSemaforo(mutexSwapFile);

	fseekSwapFile(paginaInicial,0);
	fwrite(paginaABorrar, sizeof(char), mProcSize, swapFile);

	signalSemaforo(mutexSwapFile);
}

void rellenarPaginaSwap(int nullBytesFaltantes,int numPag, int pagInicial)
{

	/*int posicionPagina= pageSize * (numPag+  paginaInicial);
	fseek(swapFile,posicionPagina,SEEK_SET);*/
	char * relleno[nullBytesFaltantes];
	memset(relleno,'\0',nullBytesFaltantes);
	int offset = pageSize - nullBytesFaltantes;
	int posRellenado = (pageSize * (numPag+  pagInicial))+offset;
	fseek(swapFile,posRellenado,SEEK_SET);

	fwrite(relleno, sizeof(char), nullBytesFaltantes, swapFile);
}

void compactadora()
{
	while(1)
	{
		compactarSwapFile();
	}
}

void  compactarSwapFile()
{
	log_info(logSwap,"Compactacion iniciada por	fragmentación externa.\n");
	dormirCompactacion();
	//log_info(logSwap,"semaforos apropiados COMPACTACION!.\n");
	waitSemaforo(mutexMProcList);
	waitSemaforo(mutexSwapFile);
	waitSemaforo(mutexArrayPaginasSwap);
	list_sort(listaPIds,(void*) numeroPaginamenor);

	nroPagLibre = buscarPrimerPaginaLibre();
	paginaOcupada = buscarPaginaOcupadaMayorQue(nroPagLibre);

	if(nroPagLibre == -1)
	{
		//NO HAY PAGINAS LIBRES
		log_info(logSwap,"Archivo Swap esta lleno, no se puede compactar.");
	}
	else if ((nroPagLibre == (cantidadPaginasSwap -1)) || (paginaOcupada == -1))
	{
		/*
		 * SI LA ULTIMA PAGINA ES LA (PRIMER Y) UNICA LIBRE
		 * O SI TODAS LAS PAGINAS DESPUES DE LA PRIMER LIBRE ENCONTRADA TAMBIEN ESTAN LIBRES
		 */

		log_info(logSwap,"Archivo Swap ya esta compactado.");
	}
	else
	{
			// ACA ES DONDE SE COMPACTA EL ARCHIVO
		t_swap_mem *mProc;
		mProc = list_find(listaPIds,(void*)mismoMProcPaginaOcupada);

		if(mProc != NULL)
		{
			mProcPaginaOcupada = mProc->numPag;
			mProcCantidadPaginasOcupadas= mProc->cantPag;
		}

		while((nroPagLibre != -1)  && (paginaOcupada != -1) && (mProc != NULL))
		{
			int tamanioMProc= pageSize * mProcCantidadPaginasOcupadas;
			char * moverPagina = malloc (tamanioMProc);
			memset(moverPagina,'\0',tamanioMProc);
			fseekSwapFile(mProcPaginaOcupada,0);
			fread(moverPagina,sizeof(char),tamanioMProc, swapFile);
			fseekSwapFile(nroPagLibre,0);
			fwrite(moverPagina,sizeof(char),tamanioMProc,swapFile);
			int i;

			for(i =0 ; i<mProcCantidadPaginasOcupadas; i++)  arrayPaginasSwap[mProcPaginaOcupada+i]= 'l';
			for(i =0 ; i<mProcCantidadPaginasOcupadas;i++)  arrayPaginasSwap[nroPagLibre+i]= 'o';
			free(moverPagina);
			nroPagLibre = buscarPrimerPaginaLibre();
			paginaOcupada = buscarPaginaOcupadaMayorQue(nroPagLibre);
			if ((mProc = list_find(listaPIds,(void*)mismoMProcPaginaOcupada)) != NULL)
			{
					mProcPaginaOcupada = mProc->numPag;
					mProcCantidadPaginasOcupadas= mProc->cantPag;
			}
		}

		free(mProc); //-> revisar, seria mejor destruyendo la estructura
		log_info(logSwap,"Compactacion finalizada por fragmentación externa.\n");
	}
	signalSemaforo(mutexArrayPaginasSwap);
	signalSemaforo(mutexSwapFile);
	signalSemaforo(mutexMProcList);

	//log_info(logSwap,"semaforos liberados COMPACTACION!.\n");
}

bool mismoMProcPaginaOcupada(t_swap_mem * mProc)
{
	return paginaOcupada == mProc->numPag;
}

bool numeroPaginamenor(t_swap_mem * mProc, t_swap_mem *otro_mProc)
{
    return mProc->numPag < otro_mProc->numPag;
}

int buscarPrimerPaginaLibre()
{
	int paginaSwap=0;
	int paginaLibre=-1;
	while((paginaLibre==-1) && paginaSwap<cantidadPaginasSwap)
	{
		if( arrayPaginasSwap[paginaSwap] =='l') paginaLibre=paginaSwap;
		paginaSwap++;
	}
	return paginaLibre;
}

int buscarPaginaOcupadaMayorQue(int nroPagLibre)
{
	int paginaOcupada=-1;
	int paginaBuscada= nroPagLibre+1;
	while((paginaOcupada==-1) && (paginaBuscada<cantidadPaginasSwap))
	{
		//printf("Pagina:%d, contenido:%c\n",paginaBuscada,arrayPaginasSwap[paginaBuscada]);
		if((arrayPaginasSwap[paginaBuscada] =='o')|| (arrayPaginasSwap[paginaBuscada] =='r')) paginaOcupada=paginaBuscada;
			paginaBuscada++;
		}
	return paginaOcupada;
}

void crearSemaforos()
{
	mutexSwapFile = crearMutex();
	mutexMProcList = crearMutex();
	mutexArrayPaginasSwap = crearMutex();
}

void destruirSemaforos()
{
	destruirSemaforo(mutexSwapFile);
	destruirSemaforo(mutexMProcList);
	destruirSemaforo(mutexArrayPaginasSwap);
}

int main()
{
    crearLog();
    cargarConfig();
    crearSemaforos();

    inicializarSwapFile();
    abirSwapFile();

    listaPIds = list_create();

    pageSize = configSwap->tam_pagina;
    cantidadPaginasSwap = configSwap->cant_paginas;

    arrayPaginasSwap = malloc(cantidadPaginasSwap);

    inicializarArray();

    paginaLeida = malloc(pageSize);
    memset(paginaLeida,'\0',pageSize);

    paginaEscrita = malloc(pageSize) ;
    memset(paginaEscrita,'\0',pageSize);

   // pthread_create(&pIdCompactadora,NULL,(void*)compactadora,NULL);
    pthread_create(&pIdServerSwap,NULL,(void*)crearServer,NULL);
    pthread_join(pIdServerSwap,NULL);
 //   pthread_join(pIdCompactadora,NULL);

    cerrarSwapFile();
    finalizarSwap();

    return 0;
}
