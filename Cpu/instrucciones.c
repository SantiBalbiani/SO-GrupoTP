/*
 * instrucciones.c
 *
 *  Created on: 17/9/2015
 *      Author: utnso
 */

#include "instrucciones.h"
#include "configuracion.h"
#include "logueos.h"

enum TipoInstruccion stringToTipo(char* palabra)//TESTEAR
{
	if (string_equals_ignore_case(palabra,"iniciar"))
	{
		return INICIAR;
	}
	if (string_equals_ignore_case(palabra,"escribir"))
	{
		return ESCRIBIR;
	}
	if (string_equals_ignore_case(palabra,"leer"))
	{
		return LEER;
	}
	if (string_equals_ignore_case(palabra,"entrada-salida"))
	{
		return ES;
	}
	if (string_equals_ignore_case(palabra,"finalizar"))
	{
		return FINALIZAR;
	}
	return INSTRUCCION_INCORRECTA;
}

char* tipoToString(enum TipoInstruccion tipo)
{
	switch(tipo)
	{
	case INICIAR:
		return("iniciar");
		break;
	case ESCRIBIR:
		return("escribir");
		break;
	case LEER:
		return("leer");
		break;
	case ES:
		return("es");
		break;
	case FINALIZAR:
		return("finalizar");
		break;
	default:
		return("Instruccion no reconocida");
		break;
	}
}

char* instruccionToString(instruccion* instruccion)
{
	char* retorno = string_new();
	enum TipoInstruccion tipo = instruccion->tipo;
	switch(tipo)
	{
	case INICIAR:
		string_append(&retorno, "Iniciar, Paginas: ");
		string_append(&retorno, string_itoa(instruccion->argumento1));
		break;
	case ESCRIBIR:
		string_append(&retorno, "Escribir, Pagina: ");
		string_append(&retorno, string_itoa(instruccion->argumento1));
		string_append(&retorno, ", Texto: ");
		string_append(&retorno, instruccion->argumento2);
		break;
	case LEER:
		string_append(&retorno, "Leer Pagina: ");
		string_append(&retorno, string_itoa(instruccion->argumento1));
		break;
	case ES:
		string_append(&retorno, "Entrada-salida, Tiempo: ");
		string_append(&retorno, string_itoa(instruccion->argumento1));
		break;
	case FINALIZAR:
		string_append(&retorno, "Finalizar");
		break;
	default:
		return("Instruccion no reconocida");
		break;
	}

	string_append(&retorno, ".");
	return retorno;
}

char* iniciar(instruccion* inst, hiloCpu* hilo, int pid,proceso* proc)
{
	char* retorno = string_new();
	string_append(&retorno, "mProc ");
	string_append(&retorno, string_itoa(pid));

	enviarPorSocket(hilo->fdDma,serializarInstruccion(inst,hilo->id,pid));
	int respuesta = recibirRespuestaIniciar(hilo->fdDma);

	if (respuesta==INICIO)
	{
		string_append(&retorno, " - Iniciado.");
	}
	else
	{
		string_append(&retorno, " - Fallo.");
		list_clean(proc->instrucciones); //limpia para q no siga ejecutando
	}

	return retorno;
}

int recibirRespuestaIniciar(int fdDma)
{
	recv(fdDma, bufferDma, sizeof(bufferDma), 0); //va a recibir 0(inicio) o 1 (fallo)

	waitSemaforo(mutexBufferDma);
	int retorno = atoi(bufferDma);
	memset(bufferDma,'\0',sizeof(bufferDma));
	signalSemaforo(mutexBufferDma);

	return retorno;
}

char* leer(instruccion* inst, hiloCpu* hilo, int pid,proceso* proc) //solo para probar que funcione y haga algo
{
	enviarPorSocket(hilo->fdDma,serializarInstruccion(inst,hilo->id,pid));
	char* recepcion = string_new();
	string_append(&recepcion,recibirRespuestaLecturaEscritura(hilo->fdDma,proc));
	char* retorno = string_new();
	string_append(&retorno, "mProc ");
	string_append(&retorno, string_itoa(pid));
	if(!string_equals_ignore_case(recepcion,"-1"))
	{
		string_append(&retorno, " - Pagina ");
		string_append(&retorno, string_itoa(inst->argumento1));
		string_append(&retorno, " leida: ");
		string_append(&retorno, recepcion);
		string_append(&retorno, ".");
	}
	else
	{
		string_append(&retorno, " finalizado por falta de marco disponible.");
	}
	return retorno;
}

char* escribir(instruccion* inst, hiloCpu* hilo, int pid,proceso* proc)
{
	enviarPorSocket(hilo->fdDma,serializarInstruccion(inst,hilo->id,pid));
	char* recepcion = string_new();
	string_append(&recepcion,recibirRespuestaLecturaEscritura(hilo->fdDma,proc));
	char* retorno = string_new();
	string_append(&retorno, "mProc ");
	string_append(&retorno, string_itoa(pid));
	if(!string_equals_ignore_case(recepcion,"-1"))
	{
		string_append(&retorno, " - Pagina ");
		string_append(&retorno, string_itoa(inst->argumento1));
		string_append(&retorno, " escrita: ");
		string_append(&retorno, inst->argumento2);
		string_append(&retorno, ".");
	}
	else
	{
		string_append(&retorno, " finalizado por falta de marco disponible.");
	}
	return retorno;
}

char* recibirRespuestaLecturaEscritura(int fdDma,proceso* proc)
{
	recv(fdDma, bufferDma, sizeof(bufferDma), 0);

	waitSemaforo(mutexBufferDma);
	char* respuesta = string_new();
	string_append(&respuesta,bufferDma);
	memset(bufferDma,'\0',sizeof(bufferDma));
	signalSemaforo(mutexBufferDma);

	if (string_equals_ignore_case(respuesta,"-1"))//testear
	{
		list_clean(proc->instrucciones); //limpia para q no siga ejecutando
		return "-1";
	}

	return respuesta;
}

char* entradaSalida(int id,int tiempo)
{
	char* retorno = string_new();
	string_append(&retorno, "mProc ");
	string_append(&retorno, string_itoa(id));
	string_append(&retorno, " en entrada-salida de tiempo ");
	string_append(&retorno, string_itoa(tiempo));
	string_append(&retorno, ".");
	return retorno;
}
char* finalizar(instruccion* inst, hiloCpu* hilo, int pid)
{
	enviarPorSocket(hilo->fdDma,serializarInstruccion(inst,hilo->id,pid));


	recv(hilo->fdDma, bufferDma, sizeof(bufferDma), 0);

	waitSemaforo(mutexBufferDma);
	memset(bufferDma,'\0',sizeof(bufferDma));
	signalSemaforo(mutexBufferDma);


	char* retorno = string_new();
	string_append(&retorno, "mProc ");
	string_append(&retorno, string_itoa(pid));
	string_append(&retorno, " - Finalizado.");
	return retorno;
}

//ejecutarInstrucciones repiten mucho codigo pero es un mal necesario
int ejecutarInstruccionesFIFO(proceso* proc, hiloCpu* hilo) //testear
{
	t_list* lInstrucciones = proc->instrucciones;
	int id = proc->id;
	int iPointer;

	for(iPointer=proc->iPointer;;iPointer++)
	{
		instruccion* inst = list_get(lInstrucciones,iPointer);
		if(inst!=NULL)
		{
			procesarInstruccion(inst,hilo,id,proc);

			if (inst->tipo==ES)
			{
				iPointer++;
				terminarRafaga(proc,iPointer);
				return inst->argumento1; //devuelve el tiempo de ES
			}
		}
		else
		{
			break;
		}
	}

	terminarRafaga(proc,iPointer);
	return 0;//no hubo ES
}

int ejecutarInstruccionesRR(proceso* proc, hiloCpu* hilo) //testear
{
	t_list* lInstrucciones = proc->instrucciones;
	int id = proc->id;
	int iPointer;

	for(iPointer=proc->iPointer;proc->quantum!=0;iPointer++)
	{
		instruccion* inst = list_get(lInstrucciones,iPointer);
		if(inst!=NULL)
		{
			proc->quantum--;

			procesarInstruccion(inst,hilo,id,proc);

			if (inst->tipo==ES)
			{
				iPointer++;
				terminarRafaga(proc,iPointer);
				return inst->argumento1; //devuelve el tiempo de ES
			}
		}
		else
		{
			break;
		}
	}

	terminarRafaga(proc,iPointer);
	return 0;//no hubo ES
}

void terminarRafaga(proceso* proc, int iPointer)
{
	loguearRafaga(proc->id);
	proc->iPointer = iPointer;
}

void procesarInstruccion(instruccion* inst,hiloCpu* hilo, int id, proceso* proc)
{
	char* retorno = string_new();
	string_append(&retorno,ejecutarInstruccion(inst,hilo,id,proc));
	loguearInstruccion(inst,id,retorno);

	list_add(proc->retornos,retorno); //ejecutar instruccion devuelve el retorno de la rafaga
}

char* serializarInstruccion(instruccion* inst, int idCpu,int pid)
{
	char* serial = string_new();
	//string_append(&serial,string_itoa(idCpu));
	//string_append(&serial, SERIALIZADOR);
	string_append(&serial, string_itoa(pid));
	string_append(&serial, SERIALIZADOR);
	string_append(&serial, tipoToString(inst->tipo));

	if (string_itoa(inst->argumento1)!=NULL)
	{
		string_append(&serial, SERIALIZADOR);
		string_append(&serial, string_itoa(inst->argumento1));

		if (!string_is_empty(inst->argumento2))
		{
			sacarComillas(inst);
			string_append(&serial, "@@");
			string_append(&serial, inst->argumento2);
		}
	}
	return serial;
}

void sacarComillas(instruccion* inst)
{
	char* arg = string_new(); //variable auxiliar
	string_append(&arg,"a"); //se agrega para splitear
	string_append(&arg,inst->argumento2);
	string_append(&arg,"b"); //se agrega para splitear
	char** componentes = string_split(arg,"\"");
	free(inst->argumento2); //se libera la mem del argumento de la instruccion porq se le va a asignar de nuevo
	inst->argumento2 = string_new();
	string_append(&inst->argumento2, componentes[1]); //se le asigna al argumento de la instruccion la variable sin comillas
}

char* ejecutarInstruccion(instruccion* inst,hiloCpu* hilo, int pid,proceso* proc)
{
	char* retorno = string_new();
	switch(inst->tipo) //pasar llamados de enviar y recibir por socket a las respectivas funciones
	{
		case INICIAR:
			string_append(&retorno,iniciar(inst,hilo,pid,proc));
			break;
		case LEER:
			string_append(&retorno,leer(inst,hilo,pid,proc));
			break;
		case ESCRIBIR:
			string_append(&retorno,escribir(inst,hilo,pid,proc));
			break;
		case ES://no serializa la instruccion, ya que no gasta memoria, por lo tanto llega hasta cpu (no va a dma)
			string_append(&retorno,entradaSalida(pid,inst->argumento1));
			break;
		case FINALIZAR:
			string_append(&retorno,finalizar(inst,hilo,pid));
			break;
		default:
			break;
	}
	dormir();
	return retorno;
}

void dormir()
{
	usleep(configCPU->retardo*1000000);
}

char* limpiarBarraN(char* linea)
{
	if (string_starts_with(linea,"\n") && !string_equals_ignore_case(linea,"\n"))
	{
		linea = string_substring_from(linea,1);
	}
	return linea;
}

char* crearArgumentoDos(char* linea, char* p0, char* p1)
{
	char* principio = string_new();
	string_append(&principio, p0);
	string_append(&principio, " ");
	string_append(&principio, p1);
	string_append(&principio, " ");
	int i = string_length(principio);
	char* palabraDos = string_new();
	string_append(&palabraDos, string_substring_from(linea, i));
	free(principio);
	return palabraDos;
}

instruccion* string_to_instruccion(char* linea)//recibe solo una linea de instruccion
{
	linea = limpiarBarraN(linea);
	char** palabras = string_split(linea," ");
	instruccion* inst = createInstruccion();
	inst->tipo = stringToTipo(palabras[0]);

	if (palabras[1]!=NULL)
	{
		inst->argumento1 = atoi(palabras[1]);
		if (palabras[2]!=NULL)
		{
			string_append(&inst->argumento2,crearArgumentoDos(linea, palabras[0], palabras[1]));
		}
	}
	if(inst->tipo==INSTRUCCION_INCORRECTA)
	{
		return NULL;
	}
	return inst;
}

t_list* string_to_instrucciones(char* contenido)//recibe todo el contenido del archivo con las intrucciones
{
	t_list* lInstrucciones = list_create();
	char** instrucciones = string_split(contenido,";"); //separa las lineas
	int i;
	for(i=0;instrucciones[i]!=NULL;i++) //recorre todas las lineas
	{
		instruccion* inst = string_to_instruccion(instrucciones[i]); //convierte la linea a una instruccion
		if(inst!=NULL)
		{
			list_add(lInstrucciones,(void*)inst); //agrega la instruccion a la lista de instrucciones
		}
	}
	return lInstrucciones;
}

void imprimirInstruccion(instruccion* inst)
{
	if (string_itoa(inst->argumento1)!=NULL)
	{
		if (inst->argumento2!=NULL)
		{
			printf("%s %d %s \n",tipoToString(inst->tipo),inst->argumento1,inst->argumento2);
		}
	else
	{
		printf("%s %d \n",tipoToString(inst->tipo),inst->argumento1);
	}
	}
	else
	{
		printf("%s\n", tipoToString(inst->tipo));
	}
}

void obtenerInstrucciones(proceso* proceso)
{
	FILE* archivo = fopen(proceso->ruta, "r");

	fseek( archivo, 0L, SEEK_END );
	long tamano = ftell(archivo);//
	fseek( archivo, 0L, SEEK_SET );

	char texto[tamano];
	fread(texto,sizeof(char),tamano,archivo);

	proceso->instrucciones = string_to_instrucciones(texto);

	fclose(archivo);
}

instruccion* createInstruccion()
{
	instruccion* inst = malloc(sizeof(instruccion));
	inst->argumento1 = 0;
	inst->argumento2 = string_new();
	return inst;
}

void destroyInstruccion(instruccion* inst)
{
	free(inst->argumento2);
	free(inst);
}




