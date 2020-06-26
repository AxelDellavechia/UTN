/*
 * globalesLfs.h
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_GLOBALESLFS_H_
#define SRC_LIB_GLOBALESLFS_H_


#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <dirent.h>
#include <sys/stat.h>
//#include <sys/types.h>
#include <sys/mman.h>
#include <sys/inotify.h>

#define LOG_PATH "../LFS_LOG.log"
#define RUTA_CONFIG_CLIENTE "../configLFS.txt"
#define FORMATO_BIN ".bin"
#define FORMATO_BIN_SIN_PUNTO "bin"
#define FORMATO_TMP "tmp"
#define FORMATO_TMP_C "tmpc"
#define PATH_BLOQUES "Bloques/"
#define PATH_TABLAS "../mnt/LISSANDRA_FS/Tables/"
#define PUNTO "."
#define DOS_PUNTOS ".."
#define TABLES "Tables/"
#define BARRA "/"
#define FALSE 0
#define TRUE 1
#define OK 1
#define ERROR -1
#define CONEXIONES_PERMITIDAS 100
#define INT (sizeof(int))
#define CHAR (sizeof(char))
#define UNSIGNED_LONG_LONG (sizeof(unsigned long long))
#define SIZE_PUNTO_MONTAJE 20
#define PATH_MAX 150
#define SIZE_TABLES 7
#define SIZE_FORMATO_BIN 7
#define CANT_MAX_BLOQUES_POR_PARTICION 50
#define MAX_LINEA 200
#define HANDSHAKE_SIZE CHAR*2
#define COMANDO_AYUDA "ayuda"
#define COMANDO_SALIR "salir"
#define COMANDO_SELECT "SELECT"
#define COMANDO_INSERT "INSERT"
#define COMANDO_CREATE "CREATE"
#define COMANDO_DESCRIBE "DESCRIBE"
#define COMANDO_DROP "DROP"
#define COMANDO_JOURNAL "JOURNAL"
#define COMANDO_ADD "ADD"
#define COMANDO_RUN "RUN"
#define COMANDO_METRICS "METRICS"
#define COMANDO_COMPACTAR "COMPACTAR"
#define COMANDO_DUMP "DUMP"
#define COMANDO_TABLAS "tablas"
#define COMANDO_REGISTROS "registros"
#define MONTAJE_TABLES "Tables/"
#define MONTAJE_BLOQUES "Bloques/"
#define MONTAJE_METADATA "Metadata/"
#define MONTAJE_METADATA_PUNTO_BIN "/Metadata.bin"

#define SUCCESSFUL "1"
#define NO_EXISTE_LA_KEY "-1"
#define NO_EXISTE_LA_TABLA "-2"
#define MEMORIA_FULL "-3"

#define CARGAR_MEMORIA 0
#define IMPRIMIR_PANTALLA 1
#define NULL "\0"

#define EVENT_SIZE  ( sizeof (struct inotify_event) + 24 )
#define BUF_LEN     ( 1024 * EVENT_SIZE )



typedef struct configLFS {
	int puertoEscucha;
	char* puntoMontaje;
	int retardo;
	int tamanio_value;
	int tiempo_dump;

} archivoConfigLFS;

typedef struct {
	int size;
	char* bloques;
} bin_tmp_t;

typedef struct {
		int fd, estado;
		pthread_t hilo;
	} t_memoria;

typedef enum {
		CONECTADO = 1,
		CONECTADO_CON_CRITERIO,
		DESCONECTADO,
	} estado_cliente;

typedef struct {
		unsigned long long timestamp;
		int key;
		char* value;
	} registro_t;

typedef struct {
		char nombre_tabla[20];
		int cant_reg;
		char consistency[4];
		int cant_partitions;
		int compaction_time;
		t_list* registros;

	} tabla_t;

typedef struct{
	  	int tamanio_bloques;
	 	char* magic_number;
	 	int cantidad_bloques;
	  }t_config_MetaData;

typedef struct{
	char* nombre_tabla;
	char* consistency;
	int partitions;
	int compaction_time;
  }t_config_MetaData_tabla;


/******************************************/

typedef struct PuntoMontaje{
	char* puntoMontajeBase;
	char* puntoMontajeTables;
	char* puntoMontajeBloques;
	char* puntoMontajeMetadata;
  }dir_puntoMontaje;

/**************************************************/
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

//VARIABLES GLOBALES
t_log* logger;
archivoConfigLFS *configFile;
int fdMemoria;
int tamanio_value;
int tamanio_pagina;
t_list* listaMemorias;
t_list* lista_tablas;
t_bitarray *bitarray;
t_config_MetaData  config_MetaData;
dir_puntoMontaje *punto_Montaje;

pthread_mutex_t mutex_file_config;
#endif /* SRC_LIB_GLOBALESLFS_H_ */
