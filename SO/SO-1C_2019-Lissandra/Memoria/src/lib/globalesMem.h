/*
 * globalesMem.h
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_GLOBALESMEM_H_
#define SRC_LIB_GLOBALESMEM_H_


#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/inotify.h>


#define LOG_PATH "../Memoria_LOG.log"
#define RUTA_CONFIG_MEM "../configMEM.txt"
#define INT (sizeof(int))
#define UNSIGNED_LONG_LONG (sizeof(unsigned long long))
#define CHAR (sizeof(char))
#define TAM_MAX_QUERY 100
#define FALSE 0
#define TRUE 1
#define OK 1
#define ERROR -1
#define CONEXIONES_PERMITIDAS 100
#define HANDSHAKE_SIZE CHAR*2
#define COMANDO_AYUDA "AYUDA"
#define COMANDO_SALIR "SALIR"
#define COMANDO_SELECT "SELECT"
#define COMANDO_INSERT "INSERT"
#define COMANDO_CREATE "CREATE"
#define COMANDO_DESCRIBE "DESCRIBE"
#define COMANDO_DROP "DROP"
#define COMANDO_JOURNAL "JOURNAL"
#define COMANDO_ADD "ADD"
#define COMANDO_METRICS "METRICS"
#define COMANDO_PAGINAS "PAGINAS"
#define COMANDO_BITMAP  "BITMAP"
#define COMANDO_SEGMENTOS  "SEGMENTOS"
#define COMANDO_MARCO  "MARCO"
#define COMANDO_GOSSIPING  "GOSSIPING"
#define COMANDO_T_GOSSIPING  "T_GOSSIPING"


#define SUCCESSFUL "1"
#define NO_EXISTE_LA_KEY "-1"
#define NO_EXISTE_LA_TABLA "-2"
#define MEMORIA_FULL "-3"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

typedef struct configMem {
	char* ipSELF;
	int puertoEscucha;
	char* ipLFS;
	int puertoLFS;
	char* ipSeeds;
	char* puertoSeeds;
	int retardoMP;
	int retardoLFS;
	int tamanioMem;
	int tiempoJournal;
	int tiempoGossping;
	int numeroMem;

} archivoConfigMemoria;


typedef struct scrpt{
	int cantidad_instrucciones,
		id_sript,
		fd_script;
	char* query;
} __attribute__((packed)) query_t;


/*** PROTOCOLO ***/
typedef enum {
	TAMANIO_VALUE = 1, 					// MEMORIA - LFS
	DESCRIBE,
	CREATE,
	TABLA_GOSSIPING,
	INSERT,
	SELECT,
	DROP,
	JOURNAL,
	FIN_DEL_PROTOCOLO
} protocolo;





//MEMORIA ESTRUCTURAS
typedef struct {
		unsigned long long timestamp;
		int key;
		char* value;
	}__attribute__((packed)) pagina_t;



typedef struct {
		int nro_pagina;
		int nro_marco;
		int bit_modificado;



	} __attribute__((packed)) tablaPagina_t;



typedef struct {
		char nombre_tabla[20];
		int cant_Paginas;
		t_list* tablaPaginas;

	} tablaSegmentos_t;

typedef struct {
		int memory_number;
		char* ip;
		int puerto;
	}__attribute__((packed)) tabla_gossiping_t;

typedef struct {
		int ocupado;
		int modificado;
		unsigned long long timestamp;
	}__attribute__((packed)) bitmap_t;

//GLOBALES
t_log* logger;
archivoConfigMemoria *configFile;
int fdEscuchaKernel,fdLfs, tamanioValue;
fd_set readfds; // conjunto maestro de descriptores de fichero para select()
int max_fd;
t_list * lista_querys;
t_list * lista_querys_consola;
pthread_mutex_t mutex_file_config;
char *memoria; // La memoria
int tamanioPagina;
bitmap_t *bitmap;
int cantidad_de_marcos;
t_list* tabla_segmentos;
t_list* tabla_gossiping;
tablaPagina_t* tabla_paginas;
pthread_mutex_t mutex_socket_lfs;

#endif /* SRC_LIB_GLOBALESMEM_H_ */
