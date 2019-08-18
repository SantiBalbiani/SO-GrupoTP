/*
 * conexiones.c
 *
 *  Created on: 28/11/2015
 *      Author: utnso
 */

#include "conexiones.h"
#include "senales.h"

void realizarConexiones()
{
	fdSWAP = crearClienteSwap();
	pthread_create(&hiloServer, NULL,(void*)crearServer,NULL);
	pthread_join(hiloServer,NULL);
}

void servidorMultiplexor(int puerto)
{
	int fdmax; // número máximo de descriptores de fichero
	int listener; // descriptor de socket a la escucha
	int newfd; // descriptor de socket de nueva conexión aceptada
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

	//Nos aseguramos de que pselect no sea interrumpido por señales
	sigemptyset(&mask);
	sigaddset (&mask, SIGUSR1);
	sigaddset (&mask, SIGUSR2);
	sigaddset (&mask, SIGPOLL);

	if (sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0) {
		perror ("sigprocmask Fallo");

	}
	// añadir listener al conjunto maestro
	FD_SET(listener, &master);

	fdmax = listener; // por ahora es éste
	// bucle principal
	for(;;)
	{
		read_fds = master; // cópialo
		if (pselect (fdmax+1,&read_fds, NULL, NULL, NULL, NULL) == -1)
		//if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
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
					if ((nbytes = recv(i, bufferCPU, sizeof(bufferCPU), 0)) <= 0)
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
						if (FD_ISSET(i, &master))
						{
							char buff[2000];
							strcpy(buff,bufferCPU);

							t_contexto* contexto = malloc(sizeof(t_contexto));
							contexto->fd = i;
							contexto->buff = buff;
							pthread_t hiloInstruccion;
							pthread_create(&hiloInstruccion, NULL,(void*)procesarMensajeRecibido,contexto);
							pthread_join(hiloInstruccion, NULL);

							memset(bufferCPU,'\0',sizeof(bufferCPU));
							memset(bufferSWAP,'\0',sizeof(bufferSWAP));
							memset(buff,'\0',sizeof(buff));
						}
					}
				}
			}
		}
	}
}

void crearServer()
{
	servidorMultiplexor(configAdm->puertoEscucha);
}

int crearClienteSwap()
{
	return clienteDelServidor(configAdm->ip_swap,configAdm->puertoSwap);
}
