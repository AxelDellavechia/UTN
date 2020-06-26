/*
 * globalesKernel.h
 *
 *  Created on: 8/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_GLOBALESKERNEL_H_
#define SRC_LIB_GLOBALESKERNEL_H_


#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <sys/inotify.h>

//DEFINES
#define LOG_PATH "../KERNEL_LOG.log"
#define RUTA_CONFIG_KERNEL "../configKernel.txt"
#define FALSE 0
#define TRUE 1
#define ERROR -1
#define CHAR (sizeof(char))
#define MAX_CONSOLA 150
#define HANDSHAKE_SIZE CHAR*2
#define INT (sizeof(int))
#define UNSIGNED_LONG_LONG (sizeof(unsigned long long))
#define MAX_LINEA 200
#define COMANDO_AYUDA "AYUDA"
#define COMANDO_SALIR "SALIR"
#define COMANDO_SELECT "SELECT"
#define COMANDO_INSERT "INSERT"
#define COMANDO_CREATE "CREATE"
#define COMANDO_DESCRIBE "DESCRIBE"
#define COMANDO_DROP "DROP"
#define COMANDO_JOURNAL "JOURNAL"
#define COMANDO_ADD "ADD"
#define COMANDO_RUN "RUN"
#define COMANDO_METRICS "METRICS"
#define COMANDO_SC "SC"
#define COMANDO_SHC "SHC"
#define COMANDO_EC "EC"
#define COMANDO_TABLAS "TABLAS"
#define COMANDO_MEMORIAS "MEMORIAS"
#define COMANDO_GOSSIPING "GOSSIPING"
#define COMANDO_T_GOSSIPING "T_GOSSIPING"

#define SUCCESSFUL "1"
#define NO_EXISTE_LA_KEY "-1"
#define NO_EXISTE_LA_TABLA "-2"
#define MEMORIA_FULL "-3"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )

typedef struct configMem {
	char* ipMemoria;
	int puertoMemoria;
	int quantum;
	int multiprocesamiento;
	int refreshMetadata;
	int retardoCicloEjecucion;
	int retardoGossiping;

} archivoConfigKernel;

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


typedef struct {
	int memory_number;
	char* ip;
	int puerto;
}__attribute__((packed)) memoria_t;

typedef struct{
	char* nombre_tabla;
	char* consistency;
	int partitions;
	int compaction_time;
}t_config_MetaData_tabla;

typedef struct{
	char* instruciones[100];
	int ip;
	int cant_instrucciones;
} pcb_t;

typedef struct{
	int nro_memoria;
	int cant_inserts_y_selects;
} memorias_metrics_t;

typedef struct{
	unsigned long long tiempo_reads;
	unsigned long long tiempo_writes;
	int cant_reads;
	int cant_writes;
	int cant_inserts_select_total;
	t_list*  memorias_metrics;

} metricas_t;


//Globales
int exitFlag; // exit del programa
t_log* logger;
int fdMem;
archivoConfigKernel *configFile;
sem_t mutex_conexion;
sem_t contador_cola_listos;
pthread_mutex_t mutex_cola_listos;
pthread_mutex_t mutex_lista_tablas;
pthread_mutex_t mutex_metricas;
pthread_mutex_t mutex_file_config;

t_list* lista_tablas;
t_list* lista_memorias;
t_list* lista_sc;
t_list* lista_shc;
t_queue* cola_ec;
t_list* lista_scripts;
t_queue * cola_listos;
metricas_t* metricas;

#endif /* SRC_LIB_GLOBALESKERNEL_H_ */
