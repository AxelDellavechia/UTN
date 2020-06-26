#include "Generales.h"

process* init(int pid) {
	process* pcb = crearPCB(pid);
	t_list* listaReady = list_create();
	return pcb;
	//iniciarEstructuras();
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
	//bitarray_destroy(bitarray);
	log_destroy(logger);
	free(comando);

	destruir_semaforos();

	pthread_detach(hilo_servidor);
	pthread_detach( pthread_self() );
	pthread_cancel(hilo_servidor);
	//exit(0);
}

void crearHilos() {


	hilo_servidor= 0;
	hilo_consola= 0;
	pthread_create(&hilo_servidor, NULL, (void*) servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*) consola, NULL);

	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);

}

void iniciarEstructuras() {
	t_list* listaNew = list_create();
	t_list* listaBlocked = list_create();
	t_list* listaExit = list_create();
	t_list* listaExec = list_create();

}

void crearLoggerSUSE() {
	char * archivoLog = strdup("SUSE_LOG.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}

void inicializar_semaforos(){
	pthread_mutex_init(&mxSocketsFD,NULL);
}

void destruir_semaforos(){
	pthread_mutex_destroy(&mxSocketsFD);
}

void servidor() {

	comandoIn = nuevoSocket();
	asociarSocket(comandoIn, configFile->listenPort);
	escucharSocket(comandoIn, CONEXIONES_PERMITIDAS);

	fd_set setAux;
	int maxFD,i,socket_nuevo;

	FD_ZERO(&setMaestro); 	// borra los conjuntos maestro y temporal
	FD_ZERO(&setAux);

	maxFD = comandoIn; //Llevo control del FD maximo de los sockets
	FD_SET(comandoIn, &setMaestro); //agrego el FD del socketEscucha al setMaestro


	log_info(logger," Escuchando conexiones. Socket: %d\n",comandoIn);

	while(TRUE) {

		setAux = setMaestro;

				if (select((maxFD + 1), &setAux, NULL, NULL, NULL ) == -1) {
					printf("Error en la escucha.\n");
					log_error(logger,"Error en la escucha.\n" );
					return;
				}

		int conexionNueva = 0;

		int *comandoNuevo= reservarMemoria(INT);

		for (i = 0; i <= maxFD; i++) {
					pthread_mutex_lock(&mxSocketsFD);
					int check = FD_ISSET(i,&setAux);
					pthread_mutex_unlock(&mxSocketsFD);
					if (check) { // Me fijo en el set de descriptores a ver cual respondió
						if (i == comandoIn) { //Tengo un nuevo hilo de Sac Cli queriendose conectar
							//Esta funcion acepta una nueva conexion de Sac Cli
							//y agrega un nuevo nodo a la lista de scripts con el nuevo socket

							log_info(logger,"Nuevo Cliente Conectado");
							*comandoNuevo = aceptarConexionSocket(comandoIn);
							if (*comandoNuevo == -1){
								return;
							}

							conexionNueva = handshake_servidor(*comandoNuevo, "Planificador");
							if( validar_conexion(conexionNueva, 0) == FALSE ) {
									pthread_mutex_lock(&mxSocketsFD); //desbloquea el semaforo
									FD_CLR(i, &setMaestro); // borra el file descriptor del set
									pthread_mutex_unlock(&mxSocketsFD);
									cerrarSocket(i);

									continue; // vuelve al inicio del while
							}else{
									pthread_mutex_lock(&mxSocketsFD);
									FD_SET(*comandoNuevo, &setMaestro); //agrego el nuevo socket al setMaestro
									pthread_mutex_unlock(&mxSocketsFD);
									if (*comandoNuevo > maxFD) maxFD = *comandoNuevo;
						}
						}else { // Hay actividad nueva en algún hilo de kernel
							//SI RECIBO TRUE EN CONEXIONNUEVA, ABRO UN HILO POR CADA UNO QUE SE CONECTE
							pthread_t hilo_planificador;

							pthread_create(&hilo_planificador, NULL, (void*) thread_planificador, i);
							//pthread_join(hilo_SacCli, NULL);
						}
					}
		}
		//free(comandoNuevo);
	}
}

int thread_planificador(int fdCliente) {


	//int iterar = 1;

//	while(iterar > 0){
	aplicar_protocolo_recibir(fdCliente); // recibo mensajes
	//cerrarSocket(fdCliente);
	pthread_detach( pthread_self() );
	return FALSE;

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

int validar_cliente(char *id) {
	if( !strcmp(id, "M")) {
		log_info(logger,"Cliente aceptado");
		return TRUE;
	} else {
		printf("Cliente rechazado\n");
		return FALSE;
	}
}

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

void* reservarMemoria(int size) {
	void *puntero = malloc(size);
	if (puntero == NULL) {
		fprintf(stderr, "Error al reservar %d bytes de memoria\n", size);
		exit(ERROR);
	}
	return puntero;
}

void leerArchivoDeConfiguracion(char *ruta) {

	configFile = reservarMemoria(sizeof(archivoConfigSUSE));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "SUSE: Leyendo Archivo de Configuracion..");
		if (config_has_property(config, "LISTEN_PORT")) {
			configFile->listenPort = config_get_string_value(config, "LISTEN_PORT");
				log_info(logger, "Se encontró y cargó el contenido de LISTEN_PORT a la estructura de SUSE. Valor: %s",configFile->listenPort);
		} else {
			log_error(logger,"El archivo de configuracion no contiene el LISTEN_PORT con el parametro LISTEN_PORT");
		}
		if (config_has_property(config, "METRICS_TIMER")) {
			metricsTimer = config_get_int_value(config,	"METRICS_TIMER");
			log_info(logger, "Se encontró y cargó el contido del METRICS_TIMER a la estructura de SUSE. Valor: %d",metricsTimer);
		} else {
			log_error(logger,"El archivo de configuracion no contiene el METRICS_TIMER  con el parametro METRICS_TIMER");
		}
		if (config_has_property(config, "MAX_MULTIPROG")) {
			maxMultiprog = config_get_int_value(config,	"MAX_MULTIPROG");
			log_info(logger, "Se encontró y cargó el contido del MAX_MULTIPROG a la estructura de SUSE. Valor: %d",maxMultiprog);
		} else {
			log_error(logger, "El archivo de configuracion no contiene el MAX_MULTIPROG  con el parametro MAX_MULTIPROG");
		}
		if (config_has_property(config, "SEM_IDS")) {
			//configFile->semIDs = (char *) malloc(sizeof config_get_array_value(config, "SEM_IDS"));
			semIDs = config_get_array_value(config, "SEM_IDS");
			log_info(logger, "Se encontró y cargó el contido del SEM_IDS a la estructura de SUSE. Valor: %p", semIDs);
		} else {
			log_error(logger, "El archivo de configuracion no contiene el SEM_IDS  con el parametro SEM_IDS");
		}
		if (config_has_property(config, "SEM_INIT")) {
			semInit = config_get_array_value(config, "SEM_INIT");
			log_info(logger, "Se encontró y cargó el contido del SEM_INIT a la estructura de SUSE. Valor: %d",semInit);
		} else {
			log_error(logger, "El archivo de configuracion no contiene el SEM_INIT  con el parametro SEM_INIT");
		}
		if (config_has_property(config, "SEM_MAX")) {
			semMax = strdup(config_get_array_value(config, "SEM_MAX"));
			log_info(logger, "Se encontró y cargó el contido del SEM_MAX a la estructura de SUSE. Valor: %d",semMax);
		} else {
			log_error(logger,"El archivo de configuracion no contiene el SEM_MAX  con el parametro SEM_MAX");
		}
		if (config_has_property(config, "ALPHA_SJF")) {
			configFile->alphaSJF = config_get_int_value(config,	"ALPHA_SJF");
			log_info(logger, "Se encontró y cargó el contido del ALPHA_SJF a la estructura de SUSE. Valor: %d",configFile->alphaSJF);
		} else {
		log_error(logger, "El archivo de configuracion no contiene el ALPHA_SJF  con el parametro ALPHA_SJF");
		}
		config_destroy(config); // Libero la estructura archivoConfig
		log_info(logger,"El archivo de configuración ha sido leído correctamente");
	}
}

// **********************************
// *    Enviar y Recibir Datos	    *
// **********************************

// Enviar algo a través de sockets
int enviarPorSocket(int fdCliente, const void * mensaje, int tamanioBytes) {
	int bytes_enviados;
	int totalBytes = 0;

	while (totalBytes < tamanioBytes) {
		bytes_enviados = send(fdCliente, mensaje + totalBytes, tamanioBytes, MSG_NOSIGNAL); // El 0 significa que no le paso ningún Flag
/* send: devuelve el múmero de bytes que se enviaron en realidad, pero como estos podrían ser menos
 * de los que pedimos que se enviaran, realizamos la siguiente validación: */

		if (bytes_enviados == ERROR) {
			break;
		}
		totalBytes += bytes_enviados;
		tamanioBytes -= bytes_enviados;
	}
	if (bytes_enviados == ERROR) printf("[SOCKETS] No se pudo enviar correctamente los datos.\n");

	return bytes_enviados; // En caso de éxito, se retorna la cantidad de bytes realmente enviada
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
	log_info(logger, "Bienvenida: %s\n",buffer);
	int enviados = enviarPorSocket(fdReceptor, buffer, tamanioTotalAEnviar);

	free(mensajeSerealizado); mensajeSerealizado = NULL;
	free(buffer); buffer = NULL;

	return enviados;
}

// Recibir algo a través de sockets
int recibirPorSocket(int fdServidor, void * buffer, int tamanioBytes) {
	int total = 0;
	int bytes_recibidos;

	while (total < tamanioBytes){

	bytes_recibidos = recv(fdServidor, buffer+total, tamanioBytes, MSG_WAITALL);
	// MSG_WAITALL: el recv queda completamente bloqueado hasta que el paquete sea recibido completamente

	if (bytes_recibidos == ERROR) { // Error al recibir mensaje
		perror("[SOCKETS] No se pudo recibir correctamente los datos.\n");
		break;
			}

	if (bytes_recibidos == 0) { // Conexión cerrada
		printf("[SOCKETS] La conexión fd #%d se ha cerrado.\n", fdServidor);
		break;
	}
	total += bytes_recibidos;
	tamanioBytes -= bytes_recibidos;
		}
	return bytes_recibidos; // En caso de éxito, se retorna la cantidad de bytes realmente recibida
}

process* crearPCB(int p_id) {
	process* pcb = malloc(sizeof(process));
	t_list* ults = list_create();
	int pid = p_id;
	unsigned long long tiempoExec = 0;
	return pcb;
}

int aplicar_protocolo_recibir(int fdEmisor){

	// Validar contra NULL al recibir en cada módulo.
	// Recibo primero el head:

	int head = 0;
	int pid = 0;
	int tid = 0;
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
	// = deserealizar(head, mensaje, *tamanioMensaje);
	void * buffer = deserealizar(head, mensaje, *tamanioMensaje);
	if (buffer != NULL) {
		log_info(logger, "head: %i\n",head);
		switch(head){
			case INIT: {
				printf("Recibí INIT");
				init(pid);
			break;
			}
			case CREATE:{
				printf("Recibí CREATE");
				create(pid, tid);
			break;
			}
			case SCHEDULE_NEXT:{
				printf("Recibí SCHEDULE_NEXT");
				int tidToExecute = schedule_next();
				aplicar_protocolo_enviar(fdEmisor, SCHEDULE_NEXT, tidToExecute); //retorno OK o CODIGO DE ERROR
			break;
			}
			case JOIN:{
				printf("Recibí JOIN");
				join(buffer);
				aplicar_protocolo_enviar(fdEmisor, JOIN, 0); //retorno OK o ERROR
			break;
			}
			case CLOSE:{
				printf("Recibí CLOSE");
				int resultado = close(buffer);
				aplicar_protocolo_enviar(fdEmisor, CLOSE, resultado); //retorno OK o ERROR
			break;
			}
			case WAIT:{
				printf("Recibí WAIT");
				int resultado = wait(buffer);
				aplicar_protocolo_enviar(fdEmisor, WAIT, resultado); //retorno OK o ERROR
			break;
			}
			case SIGNAL:{
				printf("Recibí SIGNAL");
				int resultado = signal(buffer);  //lo ideal sería cambiarlo para que reciba una estructura
				aplicar_protocolo_enviar(fdEmisor, SIGNAL, resultado); //retorno el tamaño escrito
			break;
			}
			default:
				log_error(logger, "Instrucción no reconocida");
			break;
			}
		}
			free(tamanioMensaje); tamanioMensaje = NULL;
			free(mensaje); mensaje = NULL;
			return OK;
	}

int wait(int semaforo) {
	while(semaforo < 0) {

	}
	return semaforo--;
}

int signal(int semaforo) {
	return semaforo++;
}

int calcularTamanioMensaje(int head, void* mensaje){
	int tamanio;
	if(mensaje != NULL) {
		switch(head){
		case SIGNAL:
			tamanio = sizeof(int);
		break;
		}
		tamanio = strlen((char*) mensaje) + 1;
	}else{
		tamanio = 0;
	} // fin switch head
	return tamanio;
}

// SEREALIZAR: Del mensaje listo para enviar, al buffer
void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer = NULL;
	int castInt = (int) mensaje;

	switch(head) {
	case SIGNAL:{
		buffer = malloc(sizeof(int));
		memcpy(buffer,&castInt,sizeof(int));
	break;
	}
	default:
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
		break;
	}
	return buffer;
}

// DESEREAILZAR: Del buffer, al mensaje listo para recibir
void * deserealizar(int head, void * buffer, int tamanio){
	void * mensaje = NULL;
	switch(head){
	case SIGNAL: {
		mensaje = malloc(sizeof(int));
		memcpy(mensaje, buffer, tamanio);
	break;
	}
	default:
		mensaje = malloc(tamanio);
		memcpy(mensaje, buffer, tamanio);
		break;
	}
	return mensaje;
}
