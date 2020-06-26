#ifndef SRC_LIB_PRINCIPALESLFS_H_
#define SRC_LIB_PRINCIPALESLFS_H_

#include "Generales.h"
#include "Sockets.h"
#include "funcionesFS.h"

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

#include <string.h>


#define CHAR (sizeof(char))
#define CONEXIONES_PERMITIDAS 100
#define INT (sizeof(int))
#define HANDSHAKE_SIZE CHAR*2
#define LOG_PATH "./SacServer.log"
#define RUTA_CONFIG_MEM "./config.txt"
#define FALSE 0
#define TRUE 1
//codigos de error
#define OK 1
#define ERROR -1
#define NO_EXISTE_PATH -1
#define NO_POSEE_PADRE -2
#define NOMBRE_DEMASIADO_LARGO -3
#define NO_HAY_MAS_NODOS_LIBRES -4
#define ARCHIVO_YA_EXISTE -5
#define PATH_VACIO -6
#define DIRECTORIO_NO_VACIO -7
#define CREAR 1
#define LEER 0
#define COMANDO_SALIR "SALIR"
#define PARENT_RAIZ -2
#define DIR_VACIO 0



pthread_t hilo_servidor;
pthread_t hilo_consola;
t_log* logger;
int fdSacServer;
int puertoSacServer;
void crearHilos();
void leerArchivoDeConfiguracion(char *ruta);
void crearLogger();
void* reservarMemoria(int size);
void handshake_cliente(int sockClienteDe, char *mensaje);
int conectarConSacServer();
void sacCli();
void consola();
void servidor();
int validar_conexion(int ret, int modo) ;
int aplicar_protocolo_recibir(int fdEmisor);
int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje);
int validar_cliente(char *id) ;
fd_set setMaestro;
void * serealizar(int head, void * mensaje, int tamanio);
//void  deserealizar(int head, void * buffer, int tamanio, GPath* fileString);
//void  deserealizar_OPEN(int head, void * buffer, int tamanio, GOpen* fileOpen);
int calcularTamanioMensaje(int head, void* mensaje);
int comandoIn;


typedef struct puerto {
	int fdCliente;
	fd_set *setMaestro;

} socketHilo;
int thread_SacCli(int fdCliente);

typedef enum {
	READDIR = 1,
	GETATTR,
	OPEN,
	READ,
	CREATE,
	MKDIR,
	MKNOD,
	WRITE,
	UNLINK,
	RMDIR,
	FLUSH,
	CHOWN,
	CHMOD,
	UTIMES,
	RENAME,
	FIN_DEL_PROTOCOLO
} protocolo;

#endif /* SRC_LIB_PRINCIPALESLFS_H_ */
