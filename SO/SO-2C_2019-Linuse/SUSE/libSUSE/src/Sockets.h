#ifndef SRC_LIB_SOCKETS_H_
#define SRC_LIB_SOCKETS_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // memset
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>      // perror
#include <arpa/inet.h>  // INADDR_ANY
#include <unistd.h>     // close, usleep
#include <netdb.h> 		// gethostbyname
#include <netinet/in.h>
#include <fcntl.h> // fcntl
#include <commons/log.h>

#define FALSE 0
#define TRUE 1
#define ERROR -1 // Las system-calls de sockets retornan -1 en caso de error
#define manejarError(msjError) {perror(msjError);} // Tratamiento de errores
#define INT (sizeof(int))

/*typedef enum {
	FIN_DEL_PROTOCOLO =7
} protocolo;*/

// *******************************************************************
// *           FUNCIONES GENERALES PARA TRABAJAR CON SOCKETS		 *
// *******************************************************************

// Funciones para el Servidor
int  nuevoSocket();
struct sockaddr_in asociarSocket(int fd_socket, int puerto);
void escucharSocket(int fd_socket, int conexionesEntrantesPermitidas);
int  aceptarConexionSocket(int fd_socket);

// Función para el Cliente
int conectarSocket(int fd_socket, const char * ipDestino, int puerto);

// Función para cerrar un File Descriptor
void cerrarSocket(int fd_socket);

// Función para comprobar varios Sockets al mismo tiempo (Select)
void seleccionarSocket(int mayorValorDeFD,
		fd_set * fdListosParaLectura, fd_set * fdListosParaEscritura, fd_set * fdListosParaEjecucion,
			int* segundos, int* milisegundos);
void * deserealizar(int head, void * buffer, int tamanio);
t_log* logger;
#endif /* SRC_LIB_SOCKETS_H_ */
