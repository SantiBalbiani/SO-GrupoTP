#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     //memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      //perror
#include <arpa/inet.h>  //INADDR_ANY
#include <unistd.h>     //close  usleep
#include <netdb.h> 		//gethostbyname
#include <netinet/in.h>
#include <fcntl.h> //fcntl
#include <commons/string.h>
#include <commons/log.h>

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int crearSocket();//Se crea el Fd del socket

int bindearSocket(int socketFd, int puerto); //Asocia el socketFd al puerto deseado

int escucharEn(int socketFd/*,int colaDeConecciones*/); //Se pone empieza a escuchar en el socketFd asociado al pueto

int aceptarEntrantes(int socketFd);//Se aceptan conexion, pero ojo de a una (si un garron)

int conectarA(int socketFd, char* ipDestino, int puerto); //se conecta al una direccion a travez del socketFd

int clienteDelServidor(char *ipDestino,int puerto);

int enviarPorSocket(int socketFd,char* paquete);

//int recibirPorSocket(int socketFd,char* buff);//testear

void limpiarBuffer(char* buffer);

#endif /* SOCKETS_H_ */
