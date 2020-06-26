#ifndef SRC_PROTOCOLOS_COMUNICACION_H_
#define SRC_PROTOCOLOS_COMUNICACION_H_

#include "Generales.h"


#define INT (sizeof(int))
#define FALSE 0
#define TRUE 1
#define ERROR -1

int validar_recive(int status, int modo);
int validar_servidor_o_cliente(char *id, char* mensajeEsperado,t_log* logger);
int validar_conexion(int ret, int modo,t_log* logger);
int handshake_servidor(int sockClienteDe, char *mensajeEnviado , char *mensajeEsperado,t_log* logger) ;
int handshake_cliente(int sockClienteDe, char *mensajeEnviado , char *mensajeEsperado,t_log* logger) ;
int conectarCon(int fdServer , char * ipServer , int portServer,t_log* logger);
void *serealizar(int head, void* mensaje, int tamanio);
void *deserealizar(int head, void * buffer, int tamanio);
int calcularTamanioMensaje(int head, void* mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);
int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje);

#endif /* SRC_PROTOCOLOS_COMUNICACION_H_ */
