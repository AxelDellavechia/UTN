#include "protocolos_comunicacion.h"

int validar_recive(int status, int modo) {

	if( (status == ERROR) || (status == 0) ) {

		if(modo == 1) { // Modo terminante
			exit(ERROR);
		} else { // Modo no terminante
			return FALSE;
		}

	} else {
		return TRUE;
	}
}

int validar_servidor(char *id , char* mensajeEsperado,t_log* logger) {
	if(strcmp(id, mensajeEsperado) == 0 ) {
		log_info(logger,"Servidor aceptado.\n");
		return TRUE;
	} else {
		log_info(logger,"Servidor rechazado.\n");
		return FALSE;
	}
}

int validar_conexion(int ret, int modo,t_log* logger) {

	if(ret == ERROR) {

		if(modo == 1) { // Modo terminante
			exit(ERROR);
		} else { // Sino no es terminante
			return FALSE;
		}
	} else { // No hubo error
		log_info(logger, "Alguien se conectó\n");

		return TRUE;
	}
}

void handshake(int sockClienteDe, char *mensajeEnviado , char *mensajeEsperado,t_log* logger) {

		int handshake_esperado = malloc(strlen(mensajeEsperado) + 1);

		char *buff =  handshake_esperado;

		int status = enviarPorSocket(sockClienteDe, mensajeEnviado, strlen(mensajeEnviado) + 1 );

		validar_recive(status, 1); // Es terminante ya que la conexión es con el servidor

		recibirPorSocket(sockClienteDe, buff, handshake_esperado);

		buff[handshake_esperado-1] = '\0';

		if( validar_servidor(buff , mensajeEsperado,logger) == FALSE) {
			free(buff);
			exit(ERROR);
		} else {
			log_info(logger,"Handshake recibido: %s\n", buff);
			free(buff);

		}
}

int conectarCon(int fdServer , char * ipServer , int portServer,t_log* logger) {

		log_info(logger,"fdSacServer:%d", fdServer);
		log_info(logger,"ipServer:%s", ipServer);
		log_info(logger,"portServer:%d", portServer);

		int conexion = conectarSocket(fdServer, ipServer, portServer);

		if(conexion == ERROR){
			log_info(logger,"[SOCKETS] No se pudo realizar la conexión entre el socket y el servidor.");
			return FALSE;
		}
		else{
			log_info(logger,"[SOCKETS] Se pudo realizar la conexión entre el socket y el servidor.");
			return TRUE;
		}

}

void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer = NULL;

	switch(head) {
	// CASE 1: El mensaje es un texto (char*)
	case READDIR: case GETATTR: case OPEN: case READ: case CREATE:
	case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
	case FLUSH: case CHOWN: case CHMOD:{
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
		break;
	}

  } // fin switch head
	return buffer;
}

void * deserealizar(int head, void * buffer, int tamanio){

	void * mensaje = NULL;

	switch(head){
	// CASE 1: El mensaje es un texto (char*)
	case READDIR: case GETATTR: case OPEN: case READ: case CREATE:
	case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
	case FLUSH: case CHOWN: case CHMOD: {
		mensaje = malloc(tamanio);
		memcpy(mensaje, buffer, tamanio);
		break;
	}

 }
	return mensaje;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)


int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;

	switch(head){
		// CASE 1: El mensaje es un texto (char*)
			case READDIR: case GETATTR: case OPEN: case READ: case CREATE:
			case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
			case FLUSH: case CHOWN: case CHMOD:{
				tamanio = strlen((char*) mensaje) + 1;
				break;
			}

	} // fin switch head

	return tamanio;
}

void* aplicar_protocolo_recibir(int fdEmisor, int* head){

	// Validar contra NULL al recibir en cada módulo.
	// Recibo primero el head:
	int recibido = recibirPorSocket(fdEmisor, head, INT);

	if (*head < 1 || *head > FIN_DEL_PROTOCOLO || recibido <= 0){ // DESCONEXIÓN
		//printf("Error al recibir mensaje.\n");
		return NULL;
	}

	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = malloc(INT);
	recibido = recibirPorSocket(fdEmisor, tamanioMensaje, INT);
		if (recibido <= 0) return NULL;
	// Recibo por último el mensaje serealizado:
	void* mensaje = malloc(*tamanioMensaje);
	recibido = recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);
		if (recibido <= 0) return NULL;
	// Deserealizo el mensaje según el protocolo:
	void* buffer = deserealizar(*head, mensaje, *tamanioMensaje);

	free(tamanioMensaje); tamanioMensaje = NULL;
	free(mensaje); mensaje = NULL;

	return buffer;
}

int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 || head > FIN_DEL_PROTOCOLO) printf("Error al enviar mensaje.\n");
	// Calculo el tamaño del mensaje:
	tamanioMensaje = calcularTamanioMensaje(head, mensaje);

	// Serealizo el mensaje según el protocolo (me devuelve el mensaje empaquetado):
	void *mensajeSerealizado = serealizar(head, mensaje, tamanioMensaje);

	// Lo que se envía es: head + tamaño del msj + el msj serializado:
	tamanioTotalAEnviar = 2*INT + tamanioMensaje;

	// Meto en el buffer las tres cosas:
	void *buffer = malloc(tamanioTotalAEnviar);
	memcpy(buffer + desplazamiento, &head, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &tamanioMensaje, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensajeSerealizado, tamanioMensaje);

	// Envío la totalidad del paquete (lo contenido en el buffer):
	int enviados = enviarPorSocket(fdReceptor, buffer, tamanioTotalAEnviar);

	free(mensajeSerealizado); mensajeSerealizado = NULL;
	free(buffer); buffer = NULL;

	return enviados;
}
