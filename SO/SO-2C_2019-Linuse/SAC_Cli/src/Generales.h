#ifndef GENERALES_H
#define GENERALES_H

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <signal.h>

#include <fuse.h>

#include <limits.h>

#include "sockets.h"
#include <pthread.h>

#include "funcionesFuse.h"

#define LOG_PATH "../SacCli.log"
#define RUTA_CONFIG_MEM "../config.txt"
#define FALSE 0
#define TRUE 1
#define ERROR -1
#define CONEXIONES_PERMITIDAS 100

t_log * logger;

pthread_mutex_t semaforo;

typedef struct config {
	int puertoSacServer;
	int puertoEscucha;
	char* ipSacServer;
	char* puntoMontaje;
} archivoConfigSacCli;

/*
struct fuse_extras {
	t_log * logger;
    char  *rootdir;
};


struct fuse_extras * param ;
*/

//#define FUSE_EXTRAS ((struct fuse_extras *) fuse_get_context()->private_data)

archivoConfigSacCli *configSacCli;

int fdSacServer;
int fdSacCli;

void leerArchivoDeConfiguracion(char *ruta,t_log * logger);
void crearLogger();
void capturarError(int signal);
int capturarErroresFS ( int error );
void* reservarMemoria(int size);
unsigned long long obtener_timestamp();
int cantFileName(char* path);
void inicializar_semaforos();

//char * parse( char * buffer );
//int consolaFuse();
//void consola() ;
//void pathCompleto(char fpath[PATH_MAX], const char *path);
//char* generarPaqueteFuncion(char * operacion , char * mensaje );
//char * quitarCaracter( char * buffer , char caracter);

#endif

