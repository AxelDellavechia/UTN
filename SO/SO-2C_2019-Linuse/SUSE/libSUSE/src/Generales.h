
#ifndef LIBSUSE_SRC_GENERALES_H_
#define LIBSUSE_SRC_GENERALES_H_

#include "Sockets.h"
#include <stdio.h>
#include <commons/bitarray.h>
#include <pthread.h>
#include "implementacion.h"
#include "libsuse.h"

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/inotify.h>


#define CONEXIONES_PERMITIDAS 100
#define RUTA_CONFIG_SUSE "../SUSE_Config.txt"
#define OK 1
#define HANDSHAKE_SIZE CHAR*2
#define CHAR (sizeof(char))
#define FALSE 0
#define TRUE 1
#define ERROR -1
#define LOG_PATH "../SUSE_LOG.log"

typedef struct configSUSE {
	int listenPort;
	int metricsTimer;
	int maxMultiprog;
	char** semIDs;
	char* semInit;
	char* semMax;
	float alphaSJF;

} archivoConfigSUSE;

typedef struct{
	t_list* ults;
	int pid;
	unsigned long long tiempoExec;
	//_Bool ejecutando; //Me sirve para saber si este hilo se esta ejecutando o no
}process;

typedef struct puerto {
	int fdCliente;
	fd_set *setMaestro;
} socketHilo;


//Variables

pthread_t hilo_servidor;
pthread_t hilo_consola;
int comandoIn;
fd_set setMaestro;
pthread_mutex_t mxSocketsFD;
int maxMultiprog;
archivoConfigSUSE* configFile;
int metricsTimer;
char** semIDs;
char** semInit;
char** semMax;

//Declaracion Funciones

void consola();
void crearHilos();
void iniciarEstructuras();
void crearLoggerSUSE();
void inicializar_semaforos();
void destruir_semaforos();
void servidor();
int thread_planificador(int fdCliente);
int validar_conexion(int ret, int modo);
int validar_recive(int status, int modo);
int validar_cliente(char *id);
int handshake_servidor(int sockCliente, char *mensaje);
void* reservarMemoria(int size);
void leerArchivoDeConfiguracion(char *ruta);
int enviarPorSocket(int fdCliente, const void * mensaje, int tamanioBytes);
int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje);
int recibirPorSocket(int fdServidor, void * buffer, int tamanioBytes);
int aplicar_protocolo_recibir(int fdEmisor);
int calcularTamanioMensaje(int head, void* mensaje);
void * serealizar(int head, void * mensaje, int tamanio);
void * deserealizar(int head, void * buffer, int tamanio);
process* crearPCB(int p_id);
process* init(int pid);
int wait(int semaforo);
int signal(int semaforo);




#endif /* LIBSUSE_SRC_GENERALES_H_ */
