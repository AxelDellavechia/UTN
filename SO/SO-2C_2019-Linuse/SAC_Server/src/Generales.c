#include "Generales.h"
#include <pthread.h>

void crearLogger() {
	char * archivoLog = strdup("SacServer.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}


void crearHilos() {


	hilo_servidor= 0;
	hilo_consola= 0;
	pthread_create(&hilo_servidor, NULL, (void*) servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*) consola, NULL);

	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);

}



void leerArchivoDeConfiguracion(char* ruta) {


	t_config *config;
	char *ptrRuta = ruta;
	config = config_create(ptrRuta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "FS: Leyendo Archivo de Configuracion..");

		if (config_has_property(config, "LISTEN_PORT")) {
			puertoSacServer = config_get_int_value(config,"LISTEN_PORT");
			log_info(logger,"Se encontró y cargó el puerto de escucha: %d",puertoSacServer);
		}
		else {
					log_error(logger,"El archivo de configuracion no contiene el Puerto de escucha");

		}
	}
	config_destroy(config);
}

void* reservarMemoria(int size) {

		void *puntero = malloc(size);
		if(puntero == NULL) {
			fprintf(stderr, "Error al reservar %d bytes de memoria\n", size);
			exit(ERROR);
		}
		return puntero;
		free(puntero);
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

int validar_servidor(char *id) {
	if(strcmp(id, "saCServer") == 0 ) {
		log_info(logger,"Servidor aceptado.\n");
		return TRUE;
	} else {
		printf("Servidor rechazado.\n");
		return FALSE;
	}
}

int validar_conexion(int ret, int modo) {

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


int validar_cliente(char *id) {
	if( !strcmp(id, "M")) {
		log_info(logger,"Cliente aceptado");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}

void handshake_cliente(int sockClienteDe, char *mensaje) {

		char *buff = (char*) malloc(HANDSHAKE_SIZE);
		int status = recibirPorSocket(sockClienteDe, buff, HANDSHAKE_SIZE);
		validar_recive(status, 1); // Es terminante ya que la conexión es con el servidor

		buff[HANDSHAKE_SIZE-1] = '\0';
		if( validar_servidor(buff) == FALSE) {
			free(buff);
			exit(ERROR);
		} else {
			log_info(logger,"Handshake recibido: %s\n", buff);
			free(buff);
			enviarPorSocket(sockClienteDe, mensaje, HANDSHAKE_SIZE);
		}
}


void remueve_salto_de_linea(char* salida, char* texto) {
	strncpy(salida, texto, strlen(texto) - 1);
}

char* comando_preparado(char* comando) {
	char* comando_listo = calloc(1, strlen(comando));
	remueve_salto_de_linea(comando_listo, comando);
	return comando_listo;
}

int ejecutar_comando(char* comando) {
	int ret = 0;
	char* comando_listo = comando_preparado(comando);
	string_to_upper(comando_listo);
	char** parametros = string_n_split(comando_listo, 6, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_SALIR)) {
		// hacer que funcione el comando salir
		return ret;
	}

}

void sacCli() {

	printf("Hola! Ingresá \"salir\" para finalizar módulo\n");
	size_t buffer_size = 100;
	char* comando = (char *) calloc(1, buffer_size);

	while (!string_equals_ignore_case(comando, "salir\n")) {
		printf(">");
		int bytes_read = getline(&comando, &buffer_size, stdin);
		if (bytes_read == -1) {
			log_error(logger,"Error en getline");
		}
		if (bytes_read == 1) {
			continue;
		}
		if (ejecutar_comando(comando) == TRUE) {
			char* comando_listo = comando_preparado(comando);
			log_info(logger,"El comando %s fue ejecutado con exito", comando_listo);
		}

	}

	free(comando);
}



void consola() {

	printf("Hola! Ingresá \"salir\" para finalizar módulo y \"ayuda\" para ver los comandos\n");
	size_t buffer_size = 100; //por el momento restringido a 100 caracteres
	char* comando = (char *) calloc(1, buffer_size);

	while (!string_equals_ignore_case(comando, "salir\n")) {
		printf(">");
		int bytes_read = getline(&comando, &buffer_size, stdin);
		if (bytes_read == -1) {
			log_error(logger,"Error en getline");
		}
		if (bytes_read == 1) {
			continue;
		}
		/*if (ejecutar_comando(comando) == OK) {
			char* comando_listo = comando_preparado(comando);
			log_info(logger,"El comando %s fue ejecutado con exito", comando_listo);
		}*/

	}
	bitarray_destroy(bitarray);
	log_destroy(logger);
	free(comando);



	pthread_detach(hilo_servidor);
	pthread_detach( pthread_self() );
	pthread_cancel(hilo_servidor);
	//destruir_semaforos();
//	exit(0);
}


void servidor() {

	comandoIn = nuevoSocket();
	asociarSocket(comandoIn, puertoSacServer);
	escucharSocket(comandoIn, CONEXIONES_PERMITIDAS);

	fd_set setAux;
	int maxFD,i,socket_nuevo;

	FD_ZERO(&setMaestro); 	// borra los conjuntos maestro y temporal
	FD_ZERO(&setAux);

	maxFD = comandoIn; //Llevo control del FD maximo de los sockets
	FD_SET(comandoIn, &setMaestro); //agrego el FD del socketEscucha al setMaestro


	log_info(logger," Escuchando conexiones. Socket: %d\n",comandoIn);

	while(TRUE) {
		pthread_mutex_lock(&mxSocketsFD);
		setAux = setMaestro;
		pthread_mutex_unlock(&mxSocketsFD);

				if (select((maxFD + 1), &setAux, NULL, NULL, NULL ) == -1) {
					printf("Error en la escucha.\n");
					log_error(logger,"Error en la escucha.\n" );
					return;
				}

		int conexionNueva = 0;
		int comandoNuevo;//= reservarMemoria(INT);

		for (i = 0; i <= maxFD; i++) {
					pthread_mutex_lock(&mxSocketsFD);
					int check = FD_ISSET(i,&setAux);
					pthread_mutex_unlock(&mxSocketsFD);
					if (check) { // Me fijo en el set de descriptores a ver cual respondió
						if (i == comandoIn) { //Tengo un nuevo hilo de Sac Cli queriendose conectar
							//Esta funcion acepta una nueva conexion de Sac Cli
							//y agrega un nuevo nodo a la lista de scripts con el nuevo socket

							log_info(logger,"Nuevo Cliente Conectado");
							comandoNuevo = aceptarConexionSocket(comandoIn);
							if (comandoNuevo == -1){
								return;
							}

							conexionNueva = handshake_servidor(comandoNuevo, "SacServer");
							if( validar_conexion(conexionNueva, 0) == FALSE ) {
									pthread_mutex_lock(&mxSocketsFD); //desbloquea el semaforo
									FD_CLR(i, &setMaestro); // borra el file descriptor del set
									pthread_mutex_unlock(&mxSocketsFD);
									cerrarSocket(i);

									continue; // vuelve al inicio del while
							}else{
									pthread_mutex_lock(&mxSocketsFD);
									FD_SET(comandoNuevo, &setMaestro); //agrego el nuevo socket al setMaestro
									pthread_mutex_unlock(&mxSocketsFD);
									if (comandoNuevo > maxFD) maxFD = comandoNuevo;
						}
						}else { // Hay actividad nueva en algún hilo de kernel
							//SI RECIBO TRUE EN CONEXIONNUEVA, ABRO UN HILO POR CADA UNO QUE SE CONECTE
							pthread_t hilo_SacCli;
							pthread_mutex_lock(&mxHilos);
							pthread_create(&hilo_SacCli, NULL, (void*) thread_SacCli, i);
							pthread_mutex_unlock(&mxHilos);
							//pthread_join(hilo_SacCli, NULL);
						}
					}

		}
		//free(comandoNuevo);

	}
}

int thread_SacCli(int fdCliente) {


	//int iterar = 1;

//	while(iterar > 0){
	aplicar_protocolo_recibir(fdCliente); // recibo mensajes
	//cerrarSocket(fdCliente);
	pthread_mutex_lock(&mxHilos);
	pthread_detach( pthread_self() );
	pthread_mutex_unlock(&mxHilos);
	return FALSE;

}




// -- CONEXIONES ENTRE MÓDULOS --
int handshake_servidor(int sockCliente, char *mensaje) {

	enviarPorSocket(sockCliente, mensaje, string_length(mensaje));

	char *buff = (char*) malloc(HANDSHAKE_SIZE);
	int status = recibirPorSocket(sockCliente, buff, HANDSHAKE_SIZE);

	if( validar_recive(status, 0) == TRUE ) { // El cliente envió un mensaje

		buff[HANDSHAKE_SIZE-1] = '\0';
		if (validar_cliente(buff)) {
			log_info(logger,"Hice el handshake y me respondieron: %s\n", buff);
			//enviarPorSocket(sockCliente, "SacServer", string_length("SacServer"));
			free(buff);
			return TRUE;
		} else {
			free(buff);
			return FALSE;
		}

	} else { // Hubo algún error o se desconectó el cliente

		free(buff);
		return FALSE;
	}
	return FALSE; // No debería llegar acá pero lo pongo por el warning
}




int aplicar_protocolo_recibir(int fdEmisor){

	// Validar contra NULL al recibir en cada módulo.
	// Recibo primero el head:

	int head = 0;
	int result = 0;
	int recibido = recv(fdEmisor, &head, INT, MSG_WAITALL); //recibirPorSocket(fdEmisor, &head, INT);



	if (head < 1 || head > FIN_DEL_PROTOCOLO || recibido <= 0){ // DESCONEXIÓN
		//printf("Error al recibir mensaje.\n");
		return ERROR;
	}

	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = malloc(INT);
	recibido = recv(fdEmisor, tamanioMensaje, INT, MSG_WAITALL);// recibirPorSocket(fdEmisor, tamanioMensaje, INT);
		if (recibido <= 0) return ERROR;
	// Recibo por último el mensaje serealizado: cambiar al malloc según el head

	void* mensaje = malloc(*tamanioMensaje);
	recibido = recv(fdEmisor, mensaje, *tamanioMensaje, MSG_WAITALL);//recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);
	if (recibido <= 0) return ERROR;
	// Deserealizo el mensaje según el protocolo:
	void* buffer;// = deserealizar(head, mensaje, *tamanioMensaje);
	GPath bufferPath;
	GOpen bufferOpen;
	GPathRename bufferRename;
	if(head == OPEN){

		deserealizar_OPEN(head, mensaje, *tamanioMensaje, &bufferOpen);
		buffer = &bufferOpen;
	}else{


		deserealizar(head, mensaje, *tamanioMensaje, &bufferPath);
		buffer = &bufferPath;
	}


	if (buffer != NULL)
			{
				log_info(logger, "head: %i\n",head);


				switch(head){

							case READDIR: {
								printf("Recibí pedido READDIR del socket\n");

								char* lectura = string_new();
								int res = sac_readdir(&lectura,buffer);  //lo ideal sería cambiarlo para que reciba una estructura
								if(res>0){
									aplicar_protocolo_enviar(fdEmisor, READDIR, lectura); //retorno la lectura
									free(lectura);
								}
								else{
									aplicar_protocolo_enviar(fdEmisor,FIN_DEL_PROTOCOLO,res); //si el path no existe devuelve -1 si el dir existe pero está vacío devuelve 0
								}


								break;
							}
							case GETATTR:{
								printf("Recibí GETATTR\n");
								//log_info(logger,mensaje);
								GAttr* attr =malloc(sizeof(GAttr));
								int res= sac_getattr(buffer, attr); //enviar attr por sockets, se puede serializar cualquier cosa
								if(res != -1){
									aplicar_protocolo_enviar(fdEmisor, GETATTR, (int) attr);

								}
								else{
									aplicar_protocolo_enviar(fdEmisor,FIN_DEL_PROTOCOLO,res);
								}
								free(attr);
								break;
							}
							case OPEN:{
								printf("Recibí OPEN");
								int res = sac_open(buffer);
								aplicar_protocolo_enviar(fdEmisor, OPEN, (int) res); //retorno OK o CODIGO DE ERROR
								break;
							}
							case CREATE:{
								printf("Recibí CREATE");
								result = sac_create(buffer);
								aplicar_protocolo_enviar(fdEmisor, CREATE, (int) result); //retorno OK o ERROR
								break;
							}
							case MKDIR:{
								printf("Recibí MKDIR");
								result = sac_mkdir(buffer);
								aplicar_protocolo_enviar(fdEmisor, MKDIR, (int) result); //retorno OK o ERROR
								break;
							}
							case MKNOD:{
								printf("Recibí MKNOD");
								result = sac_mknod(buffer);
								aplicar_protocolo_enviar(fdEmisor, MKNOD, (int) result); //retorno OK o ERROR
								break;
							}
							case WRITE:{
								printf("Recibí WRITE");
								result = sac_write(buffer);  //lo ideal sería cambiarlo para que reciba una estructura
								aplicar_protocolo_enviar(fdEmisor, WRITE, (int) result); //retorno el tamaño escrito
								break;
							}
							case READ:{
								printf("Recibí READ");
								char* lectura = string_new();
								int res = sac_read(&lectura,buffer);  //lo ideal sería cambiarlo para que reciba una estructura
								if(res>0){
									aplicar_protocolo_enviar(fdEmisor, READ, lectura); //retorno la lectura
									free(lectura);
								}
								else{
									aplicar_protocolo_enviar(fdEmisor,FIN_DEL_PROTOCOLO,res);
								}

							}
							case UNLINK:{
								printf("Recibí UNLINK");
								result = sac_unlink(buffer);
								aplicar_protocolo_enviar(fdEmisor, MKNOD, (int) result); //retorno OK o ERROR
								break;
							}
							case RMDIR:{
								printf("Recibí RMDIR");
								result = sac_rmdir(buffer);
								aplicar_protocolo_enviar(fdEmisor, MKNOD, (int) result); //retorno OK o ERROR
								break;
							}
							case FLUSH:{
								printf("Recibí FLUSH");
								break;
							}
							case CHOWN:{
								printf("Recibí CHOWN");
								break;
							}
							case RENAME:{
								printf("Recibí OPEN");
								int res = sac_rename(buffer);
								aplicar_protocolo_enviar(fdEmisor, RENAME, (int) res); //retorno OK o CODIGO DE ERROR
								break;
							}
							case UTIMES:{
								printf("Recibí UTIMES");
								unsigned long long time = 0;
								time = sac_utimes(buffer);
								aplicar_protocolo_enviar(fdEmisor, UTIMES,(unsigned long long) time); //retorno OK o ERROR
								break;
							}
							default:
								log_error(logger, "Instrucción no reconocida");
								break;
						}

			}


	free(tamanioMensaje); tamanioMensaje = NULL;
	free(mensaje); mensaje = NULL;
	if(head == OPEN){
		free(bufferOpen.path);
	}else{
		free (bufferPath.path);
	}
	//free (buffer);
	pthread_mutex_lock(&mxSocketsFD);
	FD_CLR(fdEmisor,&setMaestro);
	pthread_mutex_unlock(&mxSocketsFD);
	return OK;
} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)


int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotalAEnviar;

	if (head < 1 ) printf("Error al enviar mensaje.\n");
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



int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;

	switch(head){
			//case 1:{ //El mensaje es un texto (char*)
			//case DESCRIBE: case CREATE: case TABLA_GOSSIPING: case INSERT: case SELECT: case JOURNAL: case DROP:{
		case READDIR: case READ: case FLUSH: case CHOWN: case CHMOD: {
			if(mensaje != NULL)
				tamanio = strlen((char*) mensaje) + 1;
			else
				tamanio = 0;
			//printf("El tamanio de query es %i\n",tamanio);
			break;
			}
		case GETATTR:{
			tamanio = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(unsigned long long);
			break;
		}
		case OPEN: case UNLINK: case CREATE: case FIN_DEL_PROTOCOLO: case MKDIR: case MKNOD: case WRITE: case RMDIR: case RENAME:{
			tamanio = sizeof(int);
			break;
		}
			//devuelve un unsigned long
		case UTIMES:{
			tamanio =sizeof(unsigned long long);

		}


	} // fin switch head

	return tamanio;
}

unsigned long long obtener_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = ((unsigned long long)tv.tv_sec*1e3) + ((unsigned long long)tv.tv_usec/1000);
	return timestamp;
}

void inicializar_semaforos(){


	//inicializo semaforos de nodos
	for(int i = 0; i < CANT_MAX_ARCHIVOS; i++){
		pthread_mutex_init(&mxNodos[i], NULL);
	}
	pthread_mutex_init(&mxTablaNodos, NULL);
	pthread_mutex_init(&mxNodoLibre, NULL);
	pthread_mutex_init(&mxBitmap, NULL);
	pthread_mutex_init(&mxReservaBloques,NULL);
	pthread_mutex_init(&mxSocketsFD,NULL);
	pthread_mutex_init(&mxEscriboNodo,NULL);
	pthread_mutex_init(&mxHilos,NULL);

}

void destruir_semaforos(){
	pthread_mutex_destroy(&mxNodoLibre);
	pthread_mutex_destroy(&mxTablaNodos);
	pthread_mutex_destroy(&mxBitmap);
	pthread_mutex_destroy(&mxReservaBloques);
	for(int i = 0; i<1024;i++){
		pthread_mutex_destroy(&mxNodos[i]);
	}
	pthread_mutex_destroy(&mxSocketsFD);
	pthread_mutex_destroy(&mxEscriboNodo);
	pthread_mutex_destroy(&mxHilos);
}

void  deserealizar_OPEN(int head, void * buffer, int tamanio, GOpen* fileOpen){

	void * mensaje = NULL;

	switch(head){

	case OPEN:{
		//GOpen* fileOpen = malloc(sizeof(GOpen));
		int desplazamiento = 0;
		fileOpen->path = malloc(1);
		memcpy(&fileOpen->path_tam,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(uint32_t);
		fileOpen->path = realloc(fileOpen->path,fileOpen->path_tam);
		memcpy(fileOpen->path,(buffer+desplazamiento),fileOpen->path_tam);
		desplazamiento += fileOpen->path_tam;
		memcpy(&fileOpen->create,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&fileOpen->ensure,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&fileOpen->trunc,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		free(fileOpen->path);
		//return (fileOpen);
		break;
	}
 }
}

void  deserealizar_rename(int head, void * buffer, int tamanio, GPathRename* fileRename){

	void * mensaje = NULL;
/*
	switch(head){

	case RENAME:{
		//GOpen* fileOpen = malloc(sizeof(GOpen));
		int desplazamiento = 0;
		fileRename-> = malloc(1);
		memcpy(&fileRenamefileOpen->path_tam,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(uint32_t);
		fileRename->path = realloc(fileOpen->path,fileOpen->path_tam);
		memcpy(fileRename->path,(buffer+desplazamiento),fileOpen->path_tam);
		desplazamiento += fileOpen->path_tam;
		memcpy(&fileRename->create,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&fileRenamefileOpen->ensure,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		memcpy(&fileRename->trunc,(buffer+desplazamiento),sizeof(int));
		desplazamiento += sizeof(int);
		free(fileRename);
		//return (fileOpen);
		break;
	}
 }*/
}


void  deserealizar(int head, void * buffer, int tamanio, GPath* fileString){

	void * mensaje = NULL;

	switch(head){

	case READDIR: case READ: case CREATE: case MKDIR: case MKNOD: case WRITE:
	case UNLINK: case UTIMES: case RMDIR: case FLUSH: case CHOWN: case CHMOD: case RENAME: case GETATTR:{
		//GPath* fileString = malloc(sizeof(GPath));
		int desplazamiento = 0;
		int tam;
		memcpy(&fileString->path_tam,(buffer+desplazamiento),sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		fileString->path = calloc((fileString->path_tam)+1,sizeof(char));
		if(fileString->path_tam > 1){

		//fileString->path = realloc(fileString->path,fileString->path);
		memcpy(fileString->path,(buffer+desplazamiento ),(fileString->path_tam));
		}else{
		//fileString->path = realloc(fileString->path,fileString->path);
		memcpy(fileString->path,(buffer+desplazamiento),(fileString->path_tam));

		}
		//free(fileString);
		//return (fileString);
		break;
	}


 }
	//return mensaje;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)


// SEREALIZAR: Del mensaje listo para enviar, al buffer
void * serealizar(int head, void * mensaje, int tamanio){
	GAttr* attr = (GAttr*) mensaje;
	int castInt = (int) mensaje;
	unsigned long long castULL = (unsigned long long) mensaje;
	void * buffer = NULL;

	switch(head) {
		//case 1:{ //Devuelven un string
		case READDIR:   case READ: 	   case FLUSH: case CHOWN: case CHMOD: {
			buffer = malloc(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}//devuelve una estructura GAttr
		case GETATTR:
		{
			buffer = malloc(sizeof(uint32_t) + sizeof(uint32_t) + sizeof(unsigned long long));
			int desplazamiento = 0;
			memcpy(buffer+desplazamiento,&attr->file_size,sizeof(attr->file_size));
			desplazamiento += sizeof(uint32_t);
			memcpy(buffer+desplazamiento,&attr->state,sizeof(uint8_t));
			desplazamiento += sizeof(uint8_t);
			memcpy(buffer+desplazamiento,&attr->updated,sizeof(attr->updated));
		//	free(attr);
			break;
		}
		//devuelven un int
		case OPEN: case UNLINK: case FIN_DEL_PROTOCOLO: case CREATE:  case MKDIR: case MKNOD: case WRITE: case RMDIR: case RENAME:{
			buffer = malloc(sizeof(int));
			memcpy(buffer,&castInt,sizeof(int));
			break;
		}
		//devuelve un unsigned long
		case UTIMES:{
			buffer = malloc(sizeof(unsigned long long));
			memcpy(buffer,&castULL,sizeof(unsigned long long));
			break;
		}
	}
	// fin switch head

	return buffer;
}


