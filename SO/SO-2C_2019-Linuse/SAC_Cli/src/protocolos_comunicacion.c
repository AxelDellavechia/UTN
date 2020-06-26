#include "protocolos_comunicacion.h"

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

void remueve_salto_de_linea(char* salida, char* texto) {
	strncpy(salida, texto, strlen(texto) - 1);

}

int validar_conexion(int ret, int modo,t_log* logger) {

	if(ret == ERROR) {

		if(modo == 1) { // Modo terminante
			exit(ERROR);
		} else { // Sino no es terminante
			return FALSE;
		}
	} else { // No hubo error
		log_info(logger, "Alguien se conectó");

		return TRUE;
	}
}


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

int validar_servidor_o_cliente(char *id , char* mensajeEsperado,t_log* logger) {
	if(strcmp(id, mensajeEsperado) == 0 ) {
		log_info(logger,"Servidor o Cliente aceptado.");
		return TRUE;
	} else {
		log_info(logger,"Servidor o Cliente rechazado.");
		return FALSE;
	}
}


int handshake_servidor(int sockClienteDe, char *mensajeEnviado , char *mensajeEsperado,t_log* logger) {

	enviarPorSocket(sockClienteDe, mensajeEnviado, strlen(mensajeEnviado) + 1 );

	int tamMsjEsperado = strlen(mensajeEsperado) ;

	char *buff = (char*) malloc (tamMsjEsperado) + 1 ;

	int status = recibirPorSocket(sockClienteDe, buff, tamMsjEsperado);

	if( validar_recive(status, 0) == TRUE ) { // El cliente envió un mensaje

	//	buff[tamMsjEsperado] = '\0';

		if ( validar_servidor_o_cliente(buff,mensajeEsperado,logger) ) {
			log_info(logger,"Hice el handshake y me respondieron: %s", buff);
			//free(buff);
			return TRUE;
		} else {
			//free(buff);
			return FALSE;
		}

	} else { // Hubo algún error o se desconectó el cliente

	//	free(buff);
		return FALSE;
	}

	return FALSE; // No debería llegar acá pero lo pongo por el warning
}

int handshake_cliente(int sockClienteDe, char *mensajeEnviado , char *mensajeEsperado,t_log* logger) {

	int tamMsjEsperado = strlen(mensajeEsperado) + 1 ;

	char *buff = (char*) malloc(tamMsjEsperado);

	int status = recibirPorSocket(sockClienteDe, buff, tamMsjEsperado);

	validar_recive(status, 1); // Es terminante ya que la conexión es con el servidor

	buff[tamMsjEsperado-1] = '\0';

	if( validar_servidor_o_cliente(buff,mensajeEsperado,logger) == FALSE) {
		free(buff);
		exit(ERROR);
	} else {
		log_info(logger,"Handshake recibido: %s", buff);
		free(buff);
		enviarPorSocket(sockClienteDe, mensajeEnviado, strlen(mensajeEnviado) + 1);
	}
}

int conectarCon(int fdServer , char * ipServer , int portServer,t_log* logger) {

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
/*
void * serealizar(int head, void * mensaje, int tamanio){

	GOpen* fileOpen = (GOpen*) mensaje;
	void * buffer = malloc(tamanio);

	switch(head) {
	// CASE 1: El mensaje es un texto (char*)
	case READDIR: case GETATTR:  case READ: case CREATE:
	case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
	case FLUSH: case CHOWN: case CHMOD:{
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
		break;
	}
	case OPEN:{
		//buffer = malloc( sizeof(uint32_t) +sizeof(int) * 3 +fileOpen->path_tam );
		int desplazamiento = 0;
		memcpy(buffer+desplazamiento,&fileOpen->path_tam,sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(buffer+desplazamiento,fileOpen->path,fileOpen->path_tam);
		desplazamiento += fileOpen->path_tam;
		memcpy(buffer+desplazamiento,&fileOpen->create,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer+desplazamiento,&fileOpen->ensure,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer+desplazamiento,&fileOpen->trunc,sizeof(int));
		desplazamiento += sizeof(int);
	//	free(fileOpen);
		break;

	}

  } // fin switch head
	return buffer;
}*/


void * serealizar(int head, void * mensaje, int tamanio){

	GOpen* fileOpen = (GOpen*) mensaje;

	GPath* pathString = (GPath*) mensaje;

	void * buffer = malloc(tamanio);

	GPathRename* pathRename = (GPathRename*) mensaje;

	sac_utime_file * pathUTimes = (sac_utime_file*) mensaje;

	switch(head) {

	case READDIR: case GETATTR:  case READ: case CREATE:
	case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
	case FLUSH: case CHOWN: case CHMOD:{
			int desplazamiento = 0;
			memcpy(buffer+desplazamiento,&pathString->path_tam,sizeof(uint32_t));
			desplazamiento += sizeof(uint32_t);
			memcpy(buffer+desplazamiento,pathString->path,fileOpen->path_tam);
			desplazamiento += fileOpen->path_tam;
			break;
		}

	case OPEN:{
		//buffer = malloc( sizeof(uint32_t) +sizeof(int) * 3 +fileOpen->path_tam );
		int desplazamiento = 0;
		memcpy(buffer+desplazamiento,&fileOpen->path_tam,sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(buffer+desplazamiento,fileOpen->path,fileOpen->path_tam);
		desplazamiento += fileOpen->path_tam;
		memcpy(buffer+desplazamiento,&fileOpen->create,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer+desplazamiento,&fileOpen->ensure,sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(buffer+desplazamiento,&fileOpen->trunc,sizeof(int));
		desplazamiento += sizeof(int);
	//	free(fileOpen);
		break;

	}

	case RENAME:{
		int desplazamiento = 0;
		memcpy(buffer+desplazamiento,&pathRename->path_tam,sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(buffer+desplazamiento,pathRename->path,fileOpen->path_tam);
		desplazamiento += fileOpen->path_tam;
		memcpy(buffer+desplazamiento,&pathRename->pathNew_tam,sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(buffer+desplazamiento,pathRename->pathNew,pathRename->pathNew_tam);
		desplazamiento += pathRename->pathNew_tam;
		break;
		}

	case UTIMES:{
		int desplazamiento = 0;
		memcpy(buffer+desplazamiento,&pathUTimes->path_tam,sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(buffer+desplazamiento,pathUTimes->path,pathUTimes->path_tam);
		desplazamiento += pathUTimes->path_tam;
		memcpy(buffer+desplazamiento,&pathUTimes->utime,sizeof(uint64_t));
		desplazamiento += sizeof(uint64_t);
		break;
		}

  } // fin switch head
	return buffer;
}





void * deserealizar(int head, void * buffer, int tamanio){


	void * mensaje = NULL;


	switch(head){
	// CASE 1: El mensaje es un texto (char*)
	case READDIR:  case READ: case FLUSH: case CHOWN: case CHMOD: {

		mensaje = malloc(tamanio);
		memcpy(mensaje, buffer, tamanio);
		break;
	}//reciben struct GAttr
	case GETATTR: {
		GAttr* attr = malloc(sizeof(GAttr));
		int desplazamiento = 0;
		memcpy(&attr->file_size,(buffer+desplazamiento),sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&attr->state,(buffer+desplazamiento),sizeof(uint8_t));
		desplazamiento += sizeof(uint8_t);
		memcpy(&attr->updated,(buffer+desplazamiento),sizeof(unsigned long long));
		return (attr);
		break;
	}
	//reciben int (OK o Código de error)
	case OPEN: case UNLINK: case CREATE:  case MKDIR: case MKNOD: case FIN_DEL_PROTOCOLO : case WRITE: case RMDIR: case RENAME:{
		int *respuesta = malloc(sizeof(int));
		memcpy(respuesta,buffer,sizeof(int));
		return (respuesta);
		break;
	}
		//devuelve un unsigned long
	case UTIMES:{
		unsigned long long *respuesta = malloc(sizeof(unsigned long long));
		memcpy(respuesta,buffer,sizeof(unsigned long long));
		return (respuesta);
		break;
	}


 }
	return mensaje;
}

/*int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;
	GOpen* fileOpen = (GOpen*) mensaje;

	switch(head){
		// CASE 1: El mensaje es un texto (char*)
			case READDIR:   case READ: case CREATE:
			case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
			case FLUSH: case CHOWN: case CHMOD: case GETATTR: case UTIMES:{
				tamanio = strlen((char*) mensaje) +1 ;
				break;
			}
			case OPEN:{
				tamanio = fileOpen->path_tam + sizeof(int) * 3 + sizeof(uint32_t);
				break;
			}
	} // fin switch head

	return tamanio;
}*/

int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;
	GOpen* fileOpen = (GOpen*) mensaje;
	GPath* stringPath = (GPath*) mensaje;


	switch(head){
		// CASE 1: El mensaje es un texto (char*)
			case READDIR:   case READ: case CREATE:
			case MKDIR: case MKNOD: case WRITE: case UNLINK: case RMDIR:
			case FLUSH: case CHOWN: case CHMOD: case GETATTR: case UTIMES:{
				//tamanio = strlen((char*) mensaje) +1 ;
				tamanio = strlen(stringPath->path) + 1 + sizeof(uint32_t);
				break;
			}
			case OPEN:{
				tamanio = strlen(fileOpen->path) + sizeof(int) * 3 + sizeof(uint32_t);
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
		//printf("Error al recibir mensaje.");
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
	cerrarSocket(fdSacServer);
	return buffer;
}

int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){
	fdSacServer = nuevoSocket();
	if ( conectarCon( fdSacServer ,configSacCli->ipSacServer,configSacCli->puertoSacServer,logger) )	{

		handshake_servidor(fdSacServer,"M","SacServer",logger);

		int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

		if (head < 1 || head > FIN_DEL_PROTOCOLO) printf("Error al enviar mensaje.");
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
		int enviados = enviarPorSocket(fdSacServer, buffer, tamanioTotalAEnviar);

		free(mensajeSerealizado); mensajeSerealizado = NULL;
		free(buffer); buffer = NULL;
		return enviados;

	}else{
	return ERROR;
	}
}
/*
int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 || head > FIN_DEL_PROTOCOLO) printf("Error al enviar mensaje.");
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

//	free(mensajeSerealizado); mensajeSerealizado = NULL;
	free(buffer); buffer = NULL;

	return enviados;
}
*/
