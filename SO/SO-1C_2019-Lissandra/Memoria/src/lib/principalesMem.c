/*
 * principalesMem.c
 *
 *  Created on: 12/4/2019
 *      Author: utnso
 */




#include "principalesMem.h"

void crearLoggerMemoria() {
	char * archivoLog = strdup("MEM_LOG.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}

void obtenerTamanioDeValue()
{
		int * tamValue = malloc(INT);
		recibirPorSocket(fdLfs, tamValue, INT);
		tamanioValue = *tamValue; // Seteo el tamaño de página que recibo de UMC
		free(tamValue); tamValue = NULL;
		log_info(logger, "Recibí tamanio de value: %d.", tamanioValue);

}

void handshake_cliente(int sockClienteDe, char *mensaje) {

	char *buff = (char*)reservarMemoria(HANDSHAKE_SIZE);
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

int conectarConLFS() {

	fdLfs = nuevoSocket();
	int conexion = conectarSocket(fdLfs, configFile->ipLFS, configFile->puertoLFS);
	if(conexion == ERROR){
		log_info(logger,"[SOCKETS] No se pudo realizar la conexión entre el socket y el servidor(LFS u otra Memoria).");
		return FALSE;
	}
	else{
		return TRUE;
	}

}

void armar_provisoriamente_tabla_gossiping()
{
	tabla_gossiping_t* memoria1 = (tabla_gossiping_t*) malloc(sizeof(tabla_gossiping_t));
	int tam_ip = string_length(configFile->ipSELF);
	memoria1->ip = (char*)malloc(tam_ip + 1);
	memset(memoria1->ip,'\0',tam_ip + 1);
	printf("configFile->ipSELF: %s\n",configFile->ipSELF);
	memcpy(memoria1->ip,configFile->ipSELF,tam_ip);
	memoria1->puerto = configFile->puertoEscucha;
	memoria1->memory_number = configFile->numeroMem;

	list_add(tabla_gossiping,memoria1);

	tabla_gossiping_t* memoria2 = (tabla_gossiping_t*) malloc(sizeof(tabla_gossiping_t));
	memoria2->ip = (char*)malloc(10);
	memset(memoria2->ip,'\0',10);
	memcpy(memoria2->ip,"127.0.0.1",10);
	memoria2->puerto = 8002;
	memoria2->memory_number = 2;

	list_add(tabla_gossiping,memoria2);
}

void iniciarEstructuras()
{
	lista_querys = list_create();
	tabla_segmentos = list_create();
	tabla_gossiping = list_create();

	int sizeof_memoria = configFile->tamanioMem;
	memoria = reservarMemoria(sizeof_memoria);
	memset(memoria, '\0', sizeof_memoria);

	//El tamaño de la pagina es igual a timestamp (unsigned long 4 bytes) + key( int 4 bytes) + value()

	tamanioPagina = UNSIGNED_LONG_LONG + INT + tamanioValue;
	cantidad_de_marcos = sizeof_memoria / tamanioPagina;

	printf("tamanio de value: %i\n",tamanioValue);
	printf("tamanio de la pagina: %i\n",tamanioPagina);
	printf("cantidad de marcos:  %i\n",cantidad_de_marcos);
	printf("tamanio de la memoria:  %i\n",sizeof_memoria);
	// bitmap
	bitmap = reservarMemoria(sizeof(bitmap_t) * cantidad_de_marcos);
	int i = 0;
	for(; i < cantidad_de_marcos; i++){
		bitmap[i].modificado = 0;
		bitmap[i].ocupado = 0;
		bitmap[i].timestamp = 0;
	}


	pthread_mutex_init(&mutex_socket_lfs, NULL);

	//armar_provisoriamente_tabla_gossiping();
	//Iniciar tabla de págias
//	tabla_paginas = (tablaPagina_t*) (malloc(cantidadDePaginas * tamanioPagina));
//
//	int j=0;
//	for(;j<cantidadDePaginas;j++)
//	{
//		tabla_paginas[j].bit_modificado = 0;
//		tabla_paginas[j].bit_ocupado = 0;
//		tabla_paginas[j].nro_marco = -1;
//		tabla_paginas[j].nro_pagina = -1;
//		tabla_paginas[j].nro_segmento = -1;
//	}

}


//void esperar_y_AtenderScripts()
//{
//	fdEscuchaKernel = nuevoSocket();
//	asociarSocket(fdEscuchaKernel, configFile->puertoEscucha);
//	escucharSocket(fdEscuchaKernel, CONEXIONES_PERMITIDAS);
//
//	// Bucle principal:
//	while(TRUE){
//
//		// Borra el conjunto maestro:
//		FD_ZERO(&readfds);
//
//		// Añadir listener al conjunto maestro:
//		FD_SET(fdEscuchaKernel, &readfds);
//
//		// Obtengo el descriptor de fichero mayor entre los listeners:
//		max_fd = obtenerSocketMaximoInicial();
//		int* seg = malloc(INT);
//		int* micro_seg = malloc(INT);
//		*seg = 0;
//		*micro_seg = 0;
//		seleccionarSocket(max_fd, &readfds , NULL , NULL , seg, micro_seg); // función select
//
//		if (FD_ISSET(fdEscuchaKernel, &readfds)){ // nueva conexión de Kernel
//
//						//Esta funcion acepta una nueva conexion del kernel
//						//y agrega un nuevo nodo a la lista de scripts con el nuevo socket
//			    		aceptarConexionEntranteDeKernel();
//
//
//
//		}
//		else{
//			//Recorrer la lista de scripts para atender de a una.
//			recorrer_lista_querys_y_atenderlos();
//
//		}
//
//	}
//}

char* serializar_tabla_gossiping(){
	//printf("llegue a serializar_tabla_gossiping\n");
	int i;
	t_list* lista_punteros = list_create();
	int tam_total = 0;
	for(i=0;i<list_size(tabla_gossiping);i++){
		tabla_gossiping_t* nodo = list_get(tabla_gossiping,i);
		int tam_nro_mem = string_length(string_itoa(nodo->memory_number));
		int tam_ip = strlen(nodo->ip);
		int tam_puerto = string_length(string_itoa(nodo->puerto));
		int tam_suma = tam_nro_mem + tam_ip + tam_puerto + (2 * CHAR) + 1;
		char* ptr = malloc(tam_suma);
		strcpy(ptr,string_itoa(nodo->memory_number));
		string_append(&ptr,"-");
		string_append(&ptr,nodo->ip);
		string_append(&ptr,"-");
		string_append(&ptr,string_itoa(nodo->puerto));
		string_append(&ptr,"\0");
		list_add(lista_punteros,ptr);
		//printf("valor: %s\n", ptr);
		tam_total += tam_suma;
	}

	int tam_delimitador = (list_size(tabla_gossiping)) * CHAR;
	char* buffer = malloc(tam_total + tam_delimitador);
	memset(buffer,'\0',tam_total + tam_delimitador);
	int j;
//	char* tam = string_itoa(list_size(tabla_gossiping));
//	printf("tam es: %s\n", tam);
//	string_append(&buffer,tam);
//	string_append(&buffer,"*");

	for(j=0;j<list_size(lista_punteros);j++){
		char* p = list_get(lista_punteros,j);
		string_append(&buffer,p);
		string_append(&buffer,"\n");

		free(p); p=NULL;
	}

	lista_punteros = NULL;
	//printf("el valor del buffer es: %s\n", buffer);
	return buffer;



}
//char* serializar_tabla_gossiping(){
//	printf("llegue a serializar_tabla_gossiping\n");
//	int i;
//	t_list* lista_punteros = list_create();
//	int tam_total = 0;
//	for(i=0;i<list_size(tabla_gossiping);i++){
//		tabla_gossiping_t* nodo = list_get(tabla_gossiping,i);
//		int tam_nro_mem = string_length(string_itoa(nodo->memory_number));
//		int tam_ip = strlen(nodo->ip);
//		int tam_puerto = string_length(string_itoa(nodo->puerto));
//		int tam_suma = tam_nro_mem + tam_ip + tam_puerto + (2 * CHAR);
//		char* ptr = malloc(tam_suma);
//		strcpy(ptr,string_itoa(nodo->memory_number));
//		string_append(&ptr,"-");
//		string_append(&ptr,nodo->ip);
//		string_append(&ptr,"-");
//		string_append(&ptr,string_itoa(nodo->puerto));
//		list_add(lista_punteros,ptr);
//		printf("valor: %s\n", ptr);
//		tam_total += tam_suma;
//	}
//
//	int tam_delimitador = (list_size(tabla_gossiping)) * CHAR;
//	char* buffer = malloc(tam_total + tam_delimitador);
//	int j;
//	char* tam = string_itoa(list_size(tabla_gossiping));
//	printf("tam es: %s\n", tam);
//	string_append(&buffer,tam);
//	string_append(&buffer,"*");
//
//	for(j=0;j<list_size(lista_punteros);j++){
//		char* p = list_get(lista_punteros,j);
//		string_append(&buffer,p);
//		string_append(&buffer,"/");
//
//		free(p); p=NULL;
//	}
//
//	lista_punteros = NULL;
//	printf("el valor del buffer es: %s\n", buffer);
//	return buffer;
//
//
//
//}
void enviar_tabla_gossiping(int socket_kernel){

	//printf("llegue a enviar_tabla_gossiping\n");
	char* buffer = serializar_tabla_gossiping();
	aplicar_protocolo_enviar(socket_kernel, TABLA_GOSSIPING,buffer);
}

void atender_pedido(int socket_kernel, int head, char* query){

	//printf("llegue a atender_pedido\n");
	switch(head)
	 {
		 case  TABLA_GOSSIPING: enviar_tabla_gossiping(socket_kernel);
				 break;
		 case  DESCRIBE: ejecutar_comando_describe_kernel(socket_kernel, query);
		 	 	 break;
		 case CREATE: ejecutar_comando_create_kernel(socket_kernel,query);
		 	 	 break;
		 case INSERT: ejecutar_comando_insert_kernel(socket_kernel,query);
		 		 break;
		 case SELECT: ejecutar_comando_select_kernel(socket_kernel,query);
		 	 	 break;
		 case JOURNAL: ejecutar_comando_journal_kernel(socket_kernel,query);
		 		 break;
		 case DROP: ejecutar_comando_drop_kernel(socket_kernel,query);
		 		 break;
		 default: break;
	 }
}

int esperar_y_AtenderScripts()
{
	fdEscuchaKernel = nuevoSocket();
	asociarSocket(fdEscuchaKernel, configFile->puertoEscucha);
	escucharSocket(fdEscuchaKernel, CONEXIONES_PERMITIDAS);


	fd_set setAux;
	fd_set setMaestro;
	int maxFD,i,socket_nuevo;

	FD_ZERO(&setMaestro); 	// borra los conjuntos maestro y temporal
	FD_ZERO(&setAux);

	maxFD = fdEscuchaKernel; //Llevo control del FD maximo de los sockets
	FD_SET(fdEscuchaKernel, &setMaestro); //agrego el FD del socketEscucha al setMaestro
	// Bucle principal:
	while(TRUE){
		setAux = setMaestro;

		if (select((maxFD + 1), &setAux, NULL, NULL, NULL ) == -1) {
			printf("Error en la escucha de Kernel\n");
			log_error(logger,"Error en la escucha de Kernel\n" );
			return EXIT_FAILURE;
		}

		//Recorro las conexiones en busca de interacciones nuevas
		for (i = 0; i <= maxFD; i++) {
			if (FD_ISSET(i,&setAux)) { // Me fijo en el set de descriptores a ver cual respondió
				if (i == fdEscuchaKernel) { //Tengo un nuevo hilo de kernel queriendose conectar
					//Esta funcion acepta una nueva conexion del kernel
					//y agrega un nuevo nodo a la lista de scripts con el nuevo socket
					//printf("llego un kernel\n");
					log_info(logger,"llego un kernel");
					socket_nuevo = aceptarConexionEntranteDeKernel();
					if (socket_nuevo == -1) return EXIT_FAILURE;

					FD_SET(socket_nuevo, &setMaestro); //agrego el nuevo socket al setMaestro
					if (socket_nuevo > maxFD) maxFD = socket_nuevo;

				}else { // Hay actividad nueva en algún hilo de kernel
					int *head=malloc(INT);
					char* query = aplicar_protocolo_recibir(i,head);

					//printf("valor de head: %i\n",*head);
					log_info(logger,"valor de head: %i",*head);
					//printf("query1: %s\n",query);
					log_info(logger,"query1: %s\n",query);
					atender_pedido(i, *head, query);

					FD_CLR(i, &setMaestro); // borra el file descriptor del set
					cerrarSocket(i); // cierra el file descriptor
					//resultado = funcione que devuelve bytes_recibidos
//					if (resultado == 0 ) { // si resultado==0 es porque se cerro una conexión

						//hiloDesconectado(i);

						//FD_CLR(i, &setMaestro); // borra el file descriptor del set
						//close(i); // cierra el file descriptor

					//} else {
						//if (resultado < 0)
							//return EXIT_FAILURE; //error al recibir


				}
			}
		}


	}
}

void journal(){

	int i=0;
	while(TRUE){

		usleep(configFile->tiempoJournal * 1000000);
		printf("Se empezo a ejecutar el hilo Journal: %i\n\n",i);
		ejecutar_journal();
		limpiar_toda_la_memoria();

	}

}

void consola() {

	printf("Hola! Ingresá \"salir\" para finalizar módulo y \"ayuda\" para ver los comandos\n");
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
		if (ejecutar_comando(comando) == OK) {
			char* comando_listo = comando_preparado(comando);
			log_info(logger,"El comando %s fue ejecutado con exito", comando_listo);
		}

	}

	free(comando);


}



archivoConfigMemoria *leerArchivoDeConfiguracion(char *ruta) {
	configFile = reservarMemoria(sizeof(archivoConfigMemoria));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "LFS: Leyendo Archivo de Configuracion..");


		if (config_has_property(config, "IP_SELF")) {
			configFile->ipSELF = strdup(
									config_get_string_value(config, "IP_SELF"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_SELF a la estructura de MEM. Valor: %s",configFile->ipSELF);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_SELF con el parametro IP_SELF");
		}
		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			configFile->puertoEscucha = config_get_int_value(config,
							"PUERTO_ESCUCHA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_ESCUCHA a la estrcutra de MEM. Valor: %d",configFile->puertoEscucha);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO de LFS con el parametro PUERTO_ESCUCHA");

		}

		if (config_has_property(config, "IP_FS")) {
			configFile->ipLFS = strdup(
									config_get_string_value(config, "IP_FS"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_FS a la estructura de MEM. Valor: %s",configFile->ipLFS);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_FS con el parametro IP_FS");
		}

		if (config_has_property(config, "PUERTO_FS")) {
			configFile->puertoLFS = config_get_int_value(config,
							"PUERTO_FS");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_FS a la estrcutra de MEM. Valor: %d",configFile->puertoLFS);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO_FS de MEM con el parametro PUERTO_FS");

		}

		if (config_has_property(config, "IP_SEEDS")) {
			configFile->ipSeeds = strdup(
									config_get_string_value(config, "IP_SEEDS"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_SEEDS a la estructura de MEM. Valor: %s",configFile->ipSeeds);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_SEEDS con el parametro IP_SEEDS");
		}

		if (config_has_property(config, "PUERTO_SEEDS")) {
			configFile->puertoSeeds = strdup(
											config_get_string_value(config, "PUERTO_SEEDS"));
									log_info(logger,
											"Se encontró y cargó el contenido de PUERTO_SEEDS a la estructura de MEM. Valor: %s",configFile->puertoSeeds);

								} else {
									log_error(logger,
											"El archivo de configuracion no contiene el PUERTO_SEEDS con el parametro PUERTO_SEEDS");
				}

		if (config_has_property(config, "RETARDO_MEM")) {
			configFile->retardoMP = config_get_int_value(config,
							"RETARDO_MEM");
					log_info(logger,
							"Se encontró y cargó el contido del RETARDO_MEM a la estrcutra de MEM. Valor: %d",configFile->retardoMP);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el RETARDO_MEM de MEM con el parametro RETARDO_MEM");

		}

		if (config_has_property(config, "RETARDO_FS")) {
			configFile->retardoLFS = config_get_int_value(config,
									"RETARDO_FS");
							log_info(logger,
									"Se encontró y cargó el contido del RETARDO_FS a la estrcutra de MEM. Valor: %d",configFile->retardoLFS);
						} else {

							log_error(logger,
									"El archivo de configuracion no contiene el RETARDO_FS de MEM con el parametro RETARDO_FS");

				}

		if (config_has_property(config, "TAM_MEM")) {
			configFile->tamanioMem = config_get_int_value(config,"TAM_MEM");
									log_info(logger,"Se encontró y cargó el contido del TAM_MEM a la estrcutra de MEM. Valor: %d",configFile->tamanioMem);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el TAM_MEM de MEM con el parametro TAM_MEM");

						}


		if (config_has_property(config, "RETARDO_JOURNAL")) {
			configFile->tiempoJournal = config_get_int_value(config,
											"RETARDO_JOURNAL");
									log_info(logger,
											"Se encontró y cargó el contido del RETARDO_JOURNAL a la estrcutra de MEM. Valor: %d",configFile->tiempoJournal);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el RETARDO_JOURNAL de MEM con el parametro RETARDO_JOURNAL");

						}

		if (config_has_property(config, "RETARDO_GOSSIPING")) {
			configFile->tiempoGossping = config_get_int_value(config,
													"RETARDO_GOSSIPING");
											log_info(logger,
													"Se encontró y cargó el contido del RETARDO_GOSSIPING a la estrcutra de MEM. Valor: %d",configFile->tiempoGossping);
										} else {

											log_error(logger,
													"El archivo de configuracion no contiene el RETARDO_GOSSIPING de MEM con el parametro RETARDO_GOSSIPING");

								}

		if (config_has_property(config, "MEMORY_NUMBER")) {
			configFile->numeroMem = config_get_int_value(config,
															"MEMORY_NUMBER");
													log_info(logger,
															"Se encontró y cargó el contido del MEMORY_NUMBER a la estrcutra de MEM. Valor: %d",configFile->numeroMem);
												} else {

													log_error(logger,
															"El archivo de configuracion no contiene el MEMORY_NUMBER de MEM con el parametro MEMORY_NUMBER");

										}


		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"El archivo de configuración ha sido leído correctamente");
		printf("El archivo de configuración ha sido leído correctamente\n");


	}
}


load_config_file(char *ruta) {
	//configFile = reservarMemoria(sizeof(archivoConfigMemoria));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "LFS: Leyendo Archivo de Configuracion..");


		if (config_has_property(config, "IP_SELF")) {
			configFile->ipSELF = strdup(
									config_get_string_value(config, "IP_SELF"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_SELF a la estructura de MEM. Valor: %s",configFile->ipSELF);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_SELF con el parametro IP_SELF");
		}
		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			configFile->puertoEscucha = config_get_int_value(config,
							"PUERTO_ESCUCHA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_ESCUCHA a la estrcutra de MEM. Valor: %d",configFile->puertoEscucha);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO de LFS con el parametro PUERTO_ESCUCHA");

		}

		if (config_has_property(config, "IP_FS")) {
			configFile->ipLFS = strdup(
									config_get_string_value(config, "IP_FS"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_FS a la estructura de MEM. Valor: %s",configFile->ipLFS);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_FS con el parametro IP_FS");
		}

		if (config_has_property(config, "PUERTO_FS")) {
			configFile->puertoLFS = config_get_int_value(config,
							"PUERTO_FS");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_FS a la estrcutra de MEM. Valor: %d",configFile->puertoLFS);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO_FS de MEM con el parametro PUERTO_FS");

		}

		if (config_has_property(config, "IP_SEEDS")) {
			configFile->ipSeeds = strdup(
									config_get_string_value(config, "IP_SEEDS"));
							log_info(logger,
									"Se encontró y cargó el contenido de IP_SEEDS a la estructura de MEM. Valor: %s",configFile->ipSeeds);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el IP_SEEDS con el parametro IP_SEEDS");
		}

		if (config_has_property(config, "PUERTO_SEEDS")) {
			configFile->puertoSeeds = strdup(
											config_get_string_value(config, "PUERTO_SEEDS"));
									log_info(logger,
											"Se encontró y cargó el contenido de PUERTO_SEEDS a la estructura de MEM. Valor: %s",configFile->puertoSeeds);

								} else {
									log_error(logger,
											"El archivo de configuracion no contiene el PUERTO_SEEDS con el parametro PUERTO_SEEDS");
				}

		if (config_has_property(config, "RETARDO_MEM")) {
			configFile->retardoMP = config_get_int_value(config,
							"RETARDO_MEM");
					log_info(logger,
							"Se encontró y cargó el contido del RETARDO_MEM a la estrcutra de MEM. Valor: %d",configFile->retardoMP);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el RETARDO_MEM de MEM con el parametro RETARDO_MEM");

		}

		if (config_has_property(config, "RETARDO_FS")) {
			configFile->retardoLFS = config_get_int_value(config,
									"RETARDO_FS");
							log_info(logger,
									"Se encontró y cargó el contido del RETARDO_FS a la estrcutra de MEM. Valor: %d",configFile->retardoLFS);
						} else {

							log_error(logger,
									"El archivo de configuracion no contiene el RETARDO_FS de MEM con el parametro RETARDO_FS");

				}

		if (config_has_property(config, "TAM_MEM")) {
			configFile->tamanioMem = config_get_int_value(config,"TAM_MEM");
									log_info(logger,"Se encontró y cargó el contido del TAM_MEM a la estrcutra de MEM. Valor: %d",configFile->tamanioMem);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el TAM_MEM de MEM con el parametro TAM_MEM");

						}


		if (config_has_property(config, "RETARDO_JOURNAL")) {
			configFile->tiempoJournal = config_get_int_value(config,
											"RETARDO_JOURNAL");
									log_info(logger,
											"Se encontró y cargó el contido del RETARDO_JOURNAL a la estrcutra de MEM. Valor: %d",configFile->tiempoJournal);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el RETARDO_JOURNAL de MEM con el parametro RETARDO_JOURNAL");

						}

		if (config_has_property(config, "RETARDO_GOSSIPING")) {
			configFile->tiempoGossping = config_get_int_value(config,
													"RETARDO_GOSSIPING");
											log_info(logger,
													"Se encontró y cargó el contido del RETARDO_GOSSIPING a la estrcutra de MEM. Valor: %d",configFile->tiempoGossping);
										} else {

											log_error(logger,
													"El archivo de configuracion no contiene el RETARDO_GOSSIPING de MEM con el parametro RETARDO_GOSSIPING");

								}

		if (config_has_property(config, "MEMORY_NUMBER")) {
			configFile->numeroMem = config_get_int_value(config,
															"MEMORY_NUMBER");
													log_info(logger,
															"Se encontró y cargó el contido del MEMORY_NUMBER a la estrcutra de MEM. Valor: %d",configFile->numeroMem);
												} else {

													log_error(logger,
															"El archivo de configuracion no contiene el MEMORY_NUMBER de MEM con el parametro MEMORY_NUMBER");

										}


		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"El archivo de configuración ha sido leído correctamente");
		printf("El archivo de configuración ha sido leído correctamente\n");


	}
}



void actualizar_file_config()
{

	char buffer[BUF_LEN];
	char* directorio = RUTA_CONFIG_MEM;
	while(1){
	//Creando fd para el inotify
	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		log_error(logger,"Error inotify_init");
	}
	// Creamos un monitor sobre el path del archivo de configuracion indicando que eventos queremos escuchar
	int watch_descriptor = inotify_add_watch(file_descriptor,directorio , IN_MODIFY);

	if (watch_descriptor < 0)
	{
		log_error(logger,"Error al crear el monitor..");
	}

	// Leemos la informacion referente a los eventos producidos en el file
	//la funcion read() es bloqueante, se va a desbloquear cuando se haya producido un evento sobre el archivo.
	int length = read(file_descriptor, buffer, BUF_LEN);

	if (length < 0) {
		log_error(logger,"Error al leer cambios");
	}

	int offset = 0;

	while (offset < length) {

		struct inotify_event *event = (struct inotify_event *) &buffer[offset];

		// Dentro de "mask" tenemos el evento que ocurrio y sobre donde ocurrio
		if (event->mask & IN_MODIFY) {
				//Actualizo el archivo de configuracion
				pthread_mutex_lock(&mutex_file_config);

				//int retardo_anterior=configFile->retardoCicloEjecucion;
				//int metadata_refresh_anterior=configFile->refreshMetadata;
				//int quantum_anterior = configFile->quantum;

				load_config_file(RUTA_CONFIG_MEM);



				pthread_mutex_unlock(&mutex_file_config);

				log_warning(logger,"Se modifico el archivo de configuracion");
		}
		offset += sizeof (struct inotify_event) + event->len;
		}
	//Quito el reloj del directorio
	inotify_rm_watch(file_descriptor, watch_descriptor);
	//Cierro el fd
	close(file_descriptor);
	}

}

char* quitar_corchetes(char* campo){

	/**********Le quitamos los corchetes a la lista de bloques*////////////



	int tam = strlen(campo);
	char* valor = malloc(tam);
	memcpy(valor, campo,tam);
	valor[tam - 1] = '\0';
	int p=0;
	for(;p<(tam -1);p++){
		valor[p] = valor[p+1];
	}

	//printf("valor : %s\n",valor);

	return valor;
}



 void crear_lista_con_seeds(t_list* lista_seeds){



    if(strcmp(configFile->ipSeeds,"[]") != 0){

    	char* ips = quitar_corchetes(configFile->ipSeeds);
    	char* puertos = quitar_corchetes(configFile->puertoSeeds);
    	char** ips_seeds = string_split(ips,",");
    		char** puertos_seeds = string_split(puertos,",");
    		int i;
    		for(i=0;i<10;i++){//No creo que haya mas de 10 seeds

    			if(ips_seeds[i] != NULL){
    				tabla_gossiping_t* memoria1 = (tabla_gossiping_t*) malloc(sizeof(tabla_gossiping_t));
    				char* ip_sin_comillas = quitar_comillas(ips_seeds[i]);
    				int tam_ip = string_length(ip_sin_comillas);
    				//printf("tam_ip: %i\n",tam_ip);
    				memoria1->ip = (char*)malloc(tam_ip + 1);
    				memset(memoria1->ip,tam_ip +1,'\0');

    				memcpy(memoria1->ip,ip_sin_comillas,tam_ip);
    				memoria1->ip[tam_ip] = '\0';
    				memoria1->puerto = atoi(puertos_seeds[i]);
    				list_add(lista_seeds,memoria1);
    			}else{
    				break;
    			}
    		}


    }

}



int conectarConMemoria(int fdMem, char* ip, int puerto) {

	//printf("ip: %s\n",ip);
	//printf("puerto: %i\n",puerto);
	//fdMem = nuevoSocket();
	int conexion = conectarSocket(fdMem, ip, puerto);

	if(conexion == ERROR){
		log_error(logger, "Falló conexión con Memoria.");

		return FALSE;
	}
	else{
		return TRUE;
	}

}

int calcular_cantidad_memorias(char** memorias){

	int j;
	for(j=0;j<100;j++){
		if(memorias[j] == NULL){
			//printf("cant memorias: %i\n",j);
			return j;
		}
	}
	return 100;
}

void get_tabla_gossiping(int fdMemoria,t_list* lista_gossiping_auxiliar){

	int bytes = aplicar_protocolo_enviar(fdMemoria,TABLA_GOSSIPING,NULL);
	int* head = malloc(INT);
	char* buffer = aplicar_protocolo_recibir(fdMemoria,head);

	int tam_buffer = strlen(buffer);

	buffer[tam_buffer -1] = '\0';

	char** tablas = string_split(buffer,"\n");
	int cant_mems = calcular_cantidad_memorias(tablas);
//		char** mensaje  = string_n_split(buffer, 2, "*");
//		int cabeza = atoi(mensaje[0]);
//		char* cuerpo = mensaje[1];
//		char** memorias  = string_n_split(cuerpo, cabeza, "/");
	free(head);
	int j;

	for(j=0;j<cant_mems;j++){
		char** partes = string_n_split(tablas[j], 3, "-");
		char* nro_memoria = partes[0];
		char* ip = partes[1];
		char* puerto = partes[2];

		int tam_ip = strlen(ip);

		tabla_gossiping_t* nodo = malloc(sizeof(tabla_gossiping_t)) ;
		nodo->ip = malloc(tam_ip +1);

		strcpy(nodo->ip,ip);
		nodo->puerto = atoi(puerto);
		nodo->memory_number = atoi(nro_memoria);

		list_add(lista_gossiping_auxiliar,nodo);

	}

	int i;
	for(i=0;i<list_size(lista_gossiping_auxiliar);i++){
		tabla_gossiping_t* registro = list_get(lista_gossiping_auxiliar,i);

		log_info(logger,"NRO MEMORIA: %i", registro->memory_number);
		log_info(logger,"IP: %s", registro->ip);
		log_info(logger,"PUERTO: %i", registro->puerto);

//		printf("NRO MEMORIA: %i\n", registro->memory_number);
//		printf("IP: %s\n", registro->ip);
//		printf("PUERTO: %i\n\n", registro->puerto);

	}


}

void mostrar_tabla_gossiping(){


	int y;
	for(y=0;y<list_size(tabla_gossiping);y++){
		tabla_gossiping_t* registro = list_get(tabla_gossiping,y);


		printf("NRO MEMORIA: %i\n", registro->memory_number);
		printf("IP: %s\n", registro->ip);
		printf("PUERTO: %i\n\n", registro->puerto);

	}

}

void exec_gossiping() {

	t_list* lista_seeds = list_create();
	t_list* lista_gossiping_auxiliar = list_create();
	crear_lista_con_seeds(lista_seeds);

	int i;

	for (i = 0; i<list_size(lista_seeds); i++) {
		tabla_gossiping_t* mem = list_get(lista_seeds, i);

		int fdMemoria = nuevoSocket();
		if (conectarConMemoria(fdMemoria,mem->ip,mem->puerto)){ // Conexión con Memoria
			handshake_cliente(fdMemoria, "M");

			get_tabla_gossiping(fdMemoria,lista_gossiping_auxiliar);

		}

		cerrarSocket(fdMemoria);

	}

	//printf("size lista_gossiping_auxiliar: %i\n",list_size(lista_gossiping_auxiliar));

	int j;
	for(j=0;j<list_size(tabla_gossiping);j++){


		tabla_gossiping_t* mem = list_get(tabla_gossiping, j);

		bool esMemoria(tabla_gossiping_t* mem_aux){return mem_aux->puerto == mem->puerto & strcmp(mem_aux->ip,mem->ip) == 0;}
		tabla_gossiping_t* memoria_encontrada = list_find(lista_seeds,(void*) esMemoria);

		if(memoria_encontrada == NULL){	//Significa que aun no se le hizo gossiping

			int fdMemoria = nuevoSocket();
			if (conectarConMemoria(fdMemoria,mem->ip,mem->puerto)){ // Conexión con Memoria
				handshake_cliente(fdMemoria, "M");

				get_tabla_gossiping(fdMemoria,lista_gossiping_auxiliar);

			}

			cerrarSocket(fdMemoria);
		}
	}

	int h;
	for(h=0;h<list_size(tabla_gossiping);h++){
		tabla_gossiping_t* mem3 = list_get(tabla_gossiping, h);
		free(mem3->ip);
	}

	list_clean(tabla_gossiping);

	//Agrego la memoria presente a la tabla gossiping
	tabla_gossiping_t* memoria1 = (tabla_gossiping_t*) malloc(sizeof(tabla_gossiping_t));
	int tam_ip = string_length(configFile->ipSELF);
	memoria1->ip = (char*)malloc(tam_ip + 1);
	memset(memoria1->ip,'\0',tam_ip + 1);
	//printf("configFile->ipSELF: %s\n",configFile->ipSELF);
	memcpy(memoria1->ip,configFile->ipSELF,tam_ip);
	memoria1->puerto = configFile->puertoEscucha;
	memoria1->memory_number = configFile->numeroMem;

	list_add(tabla_gossiping,memoria1);

	int k;
	for(k=0;k<list_size(lista_gossiping_auxiliar);k++){

		tabla_gossiping_t* mem2 = list_get(lista_gossiping_auxiliar, k);


		bool estaMemoria(tabla_gossiping_t* mem_aux1){return mem_aux1->memory_number ==  mem2->memory_number;}
		tabla_gossiping_t* memoria_encontrada2 = list_find(tabla_gossiping,(void*) estaMemoria);

		if(memoria_encontrada2 == NULL){
			list_add(tabla_gossiping,mem2);
		}


	}


}

//void exec_gossiping() {

	//Hago gossipoing siempre con todos los seeds.

	//cada tabla que me vayan devolviendo o voy agregando a una lista auxilir

	//Hago gossiping a cada memoeria que esta en mi tabla gossipin menos a los seeds a loc cuales ya hice gossiping.
	//Cada tabla gossipin que me devuelvan lo voy agregando a la lista auxiliar.

   //Luego de la lista auxiliar saco los repetidos

	//Vacio la tabla gossiping y luego agrego los nuevos.
//}

void initGossiping() {

	exec_gossiping();
	while (TRUE) {
		usleep(configFile->tiempoGossping * 1000);
		exec_gossiping();
	}
}
