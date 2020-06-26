/*
 * principalesLfs.c
 *
 *  Created on: 12/4/2019
 *      Author: utnso
 */


#include "principalesLfs.h"


void crearLoggerLFS() {
	char * archivoLog = strdup("LFS_LOG.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}



void crearHilos() {

	pthread_t hilo_servidor, hilo_consola, hilo_dump, hilo_inotify;

	pthread_create(&hilo_servidor, NULL, (void*) servidor, NULL);
	pthread_create(&hilo_consola, NULL, (void*) consola, NULL);
	pthread_create(&hilo_dump, NULL, (void*) dump, NULL);
	pthread_create(&hilo_inotify,NULL,(void*)actualizar_file_config,NULL);

	pthread_join(hilo_dump, NULL);
	pthread_join(hilo_inotify, NULL);
	pthread_join(hilo_servidor, NULL);
	pthread_join(hilo_consola, NULL);


}



void servidor() {

	fdMemoria = nuevoSocket();

	asociarSocket(fdMemoria, configFile->puertoEscucha);
	escucharSocket(fdMemoria, CONEXIONES_PERMITIDAS);
	log_info(logger," Escuchando conexiones");
	while(TRUE) {

		int conexionNueva = 0;
		int *fdMemoriaNueva = reservarMemoria(INT);

		while(conexionNueva == 0) {

			*fdMemoriaNueva = aceptarConexionSocket(fdMemoria);

			if( validar_conexion(*fdMemoriaNueva, 0) == FALSE ) {
				continue; // vuelve al inicio del while
			} else {
				conexionNueva = handshake_servidor(*fdMemoriaNueva, "F");
				enviarTamanioValue(*fdMemoriaNueva, configFile->tamanio_value);
			}

			//verificarDesconexionDeClientes();
		} // cuando salehay una nueva memoria


		log_info(logger," Se conecto una memoria");
		crearHiloQueAtiendeAMemoria(fdMemoriaNueva);

	}

	cerrarSocket(fdMemoria);

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

void dump()
{
	while(TRUE)
	{
		usleep(configFile->tiempo_dump *1000);
		ejecutar_dump();
	}
}

void crearHiloQueAtiendeAMemoria(int *sockCliente) {
	t_memoria* unaMemoria = malloc(sizeof(t_memoria));

	unaMemoria->fd = *sockCliente;
	unaMemoria->estado = CONECTADO;

	pthread_create(&unaMemoria->hilo, NULL, &memoria, (void*) sockCliente);

	list_add(listaMemorias, unaMemoria);
}

void iniciarEstructuras()
{
	tamanio_value = configFile->tamanio_value;
	tamanio_pagina = UNSIGNED_LONG_LONG + INT + tamanio_value;

	listaMemorias = list_create();
	lista_tablas = list_create();

	pthread_mutex_init(&mutex_file_config,NULL);
//	char* path = path_bitmap();
//	//Escribo el archivo Bitmap.bin
//	creacionDeArchivoBitmap(path,config_MetaData.cantidad_bloques);
//	printf("Se creo bitmap correctamente\n");
//	free(path);

}



void* memoria(void* fdCliente) {
	//No puedo usar logger en este hilo porque me tira violacion de segmento y no entiendo porque
	//log_info(logger," Estoy en el hilo memoria");

	log_info(logger, "Se conecto una memoria y se creo un hilo para atenderla");
	int sockMemoria = *((int*) fdCliente);
	free(fdCliente); fdCliente = NULL;

	while(TRUE) {

		int head;
		void *mensaje = NULL;
		mensaje = aplicar_protocolo_recibir(sockMemoria, &head); // recibo mensajes
		log_info(logger, "head: %i\n",head);
		log_info(logger, "mensaje recibido %s\n",mensaje);
//		printf("head: %i\n",head);
//		printf("mensaje recibido %s\n",mensaje);


		switch(head){
			// CASE 1: El mensaje es un texto (char*)
			case DESCRIBE: {
				ejecutar_comando_describe_para_memoria(mensaje, sockMemoria);
				break;
			}
			case CREATE:{
				ejecutar_comando_create_para_memoria(mensaje, sockMemoria);
				break;
			}
			case SELECT:{
				//printf("mensaje: %s\n",mensaje);
				ejecutar_comando_select_memoria(mensaje,sockMemoria);
				break;
			}
			case JOURNAL:{
				ejecutar_comando_journal(mensaje,sockMemoria);
				break;
			}
			case DROP:{
				ejecutar_comando_drop_memoria(mensaje,sockMemoria);
				break;
			}

		}

		usleep(configFile->retardo * 1000); // Retardo por operacion.

		free(mensaje);




	}

	return NULL;

}

void enviarTamanioValue(int fd, int tamanio) {
	int *msj = reservarMemoria(INT);
	*msj = tamanio;
	enviarPorSocket(fd, msj, INT);
	free(msj);
}

// -- CONEXIONES ENTRE MÓDULOS --
int handshake_servidor(int sockCliente, char *mensaje) {

	enviarPorSocket(sockCliente, mensaje, HANDSHAKE_SIZE);

	char *buff = (char*)reservarMemoria(HANDSHAKE_SIZE);
	int status = recibirPorSocket(sockCliente, buff, HANDSHAKE_SIZE);

	if( validar_recive(status, 0) == TRUE ) { // El cliente envió un mensaje

		buff[HANDSHAKE_SIZE-1] = '\0';
		if (validar_cliente(buff)) {
			log_info(logger,"Hice el handshake y me respondieron: %s\n", buff);
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

void leerArchivoDeConfiguracion(char *ruta) {

	configFile = reservarMemoria(sizeof(archivoConfigLFS));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "LFS: Leyendo Archivo de Configuracion..");



		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			configFile->puertoEscucha = config_get_int_value(config,
							"PUERTO_ESCUCHA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_ESCUCHA a la estrcutra de LFS. Valor: %d",configFile->puertoEscucha);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO de LFS con el parametro PUERTO_ESCUCHA");

		}

		if (config_has_property(config, "PUNTO_MONTAJE")) {
			configFile->puntoMontaje = strdup(
									config_get_string_value(config, "PUNTO_MONTAJE"));
							log_info(logger,
									"Se encontró y cargó el contenido de PUNTO_MONTAJE a la estructura de LFS. Valor: %s",configFile->puntoMontaje);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el PUNTO_MONTAJE con el parametro PUNTO_MONTAJE");
		}

		if (config_has_property(config, "RETARDO")) {
			configFile->retardo = config_get_int_value(config,
							"RETARDO");
					log_info(logger,
							"Se encontró y cargó el contido del RETARDO a la estrcutra de LFS. Valor: %d",configFile->retardo);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el RETARDO de LFS con el parametro RETARDO");

		}

		if (config_has_property(config, "TAMAÑO_VALUE")) {
			configFile->tamanio_value = config_get_int_value(config,
								"TAMAÑO_VALUE");
						log_info(logger,
								"Se encontró y cargó el contido del TAMAÑO_VALUE a la estrcutra de LFS. Valor: %d",configFile->tamanio_value);
					} else {

						log_error(logger,
								"El archivo de configuracion no contiene el TAMAÑO_VALUE de LFS con el parametro TAMAÑO_VALUE");

		}

		if (config_has_property(config, "TIEMPO_DUMP")) {
			configFile->tiempo_dump = config_get_int_value(config,
										"TIEMPO_DUMP");
								log_info(logger,
										"Se encontró y cargó el contido del TIEMPO_DUMP a la estrcutra de LFS. Valor: %d",configFile->tiempo_dump);
							} else {

								log_error(logger,
										"El archivo de configuracion no contiene el TIEMPO_DUMP de LFS con el parametro TIEMPO_DUMP");

		}



		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"El archivo de configuración ha sido leído correctamente");
		//printf("El archivo de configuración ha sido leído correctamente\n");

		log_info(logger,
									"el valor de puerto escucha es: %d",configFile->puertoEscucha);


	}
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
} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)






/*********** Funciones de Bitmap ********************************************************/


void bitarray_to_file(){
	char *direccionArchivoBitMap = path_bitmap();

	FILE *archivo_bitmap = fopen(direccionArchivoBitMap, "wb");
	if (archivo_bitmap == NULL)
	{
		printf("Error de apertura archivo bitmap:%s\n\n",direccionArchivoBitMap);
		fclose(archivo_bitmap);

	}

	fwrite(bitarray->bitarray, bitarray->size, 1, archivo_bitmap);
	free(direccionArchivoBitMap);
	fclose(archivo_bitmap);
}

void crearBitmap(){

	char *direccionArchivoBitMap = path_bitmap();

	int bitmap = open(direccionArchivoBitMap, O_RDWR);


	if(bitmap == ERROR){
		printf("No se pudo abrir el archivo bitmap\n");
	}

	struct stat mystat;

	if (fstat(bitmap, &mystat) < 0) {
	    printf("Error al establecer fstat\n");
	    close(bitmap);
	}
    char *bmap ;


    //Subo a memoria el contenido del archivo bitmap desde 0 hasta mystat.st_size bytes.

    bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_PRIVATE, bitmap, 0);


	if (bmap == MAP_FAILED) {
			printf("Error al mapear a memoria: %s\n", strerror(errno));

	}



	//bitarray = bitarray_create_with_mode(bmap, config_MetaData.cantidad_bloques / 8, MSB_FIRST);
	bitarray = bitarray_create_with_mode(mmap(NULL, config_MetaData.cantidad_bloques / 8, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0), config_MetaData.cantidad_bloques / 8, MSB_FIRST);

	size_t	cantidadDebits= bitarray_get_max_bit (bitarray);

	free(direccionArchivoBitMap);
	close(bitmap);

	printf("cantidadDebits: %i\n",cantidadDebits);
	int i;
	for (i=0;i<cantidadDebits;i++){

		bitarray_clean_bit(bitarray,i);

	}
/************* Lo comentado es para probar el bitarray ********/
//	int i;
//
//	for (i=0;i<cantidadDebits;i++){
//
//		bitarray_set_bit(bitarray,i);
//
//
//	}
//	bitarray_clean_bit(bitarray,1);
//
//	bitarray_clean_bit(bitarray,2);
//
//
//
//	for (i=0;i<cantidadDebits;i++){
//
//		if(bitarray_test_bit(bitarray,i) == 1){
//			printf("Bloque ocupado %d\n",i);
//		}
//
//
//	}

	//printf("Cantidad de bloques libres %d\n", cantidadDeBloquesLibres());
	//printf("Index del proximo bloque libre %d\n",proximobloqueLibre());

	/**Graba en el archivo  el bitarray*/
	//bitarray_to_file();
/***************** Lo comentado es para probar el bitarray ********/



}



int creacionDeArchivoBitmap(char *path,int cantidad){
    int x = 0;
    FILE *fh = fopen (path, "wb");
    if(fh == NULL){
    	printf("Error en el fopen\n");
    }
    int i;
    for( i=0;i<cantidad;i++){
        if (fh != NULL) {
                fwrite (&x, sizeof (x), 1, fh);
        }
    }
    fclose(fh);
    return 0;

}

/****************************************************************************/

int leer_metaData_principal(){
	char *direccionArchivoMedata=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen("/Metadata/Metadata.bin"));;

	strcpy(direccionArchivoMedata,configFile->puntoMontaje);
	string_append(&direccionArchivoMedata,"Metadata/Metadata.bin");
	printf("direccionArchivoMedata: %s\n",direccionArchivoMedata);
	t_config *archivo_MetaData;
	archivo_MetaData=config_create(direccionArchivoMedata);
	config_MetaData.cantidad_bloques=config_get_int_value(archivo_MetaData,"BLOCKS");
    config_MetaData.magic_number=string_duplicate(config_get_string_value(archivo_MetaData,"MAGIC_NUMBER"));
	config_MetaData.tamanio_bloques=config_get_int_value(archivo_MetaData,"BLOCK_SIZE");
	free(direccionArchivoMedata);
	config_destroy(archivo_MetaData);
	return 0;
}


/***************************************/

void crearBloques(void)
{
	int i;
	for (i=0; i< config_MetaData.cantidad_bloques;i++ )
	{
			FILE *block;
			char* bloque = (char*) malloc(string_length(punto_Montaje->puntoMontajeBloques)+ string_length(string_itoa(i))+ string_length(".bin"));
			strcpy(bloque,punto_Montaje->puntoMontajeBloques);
			strcat(bloque,string_itoa(i));
			strcat(bloque,".bin");
			block = fopen(bloque,"a");
			fclose(block);
			free(bloque);
	}
}

void crearPuntoMontaje(void)
{
		punto_Montaje = (struct PuntoMontaje *) malloc (sizeof(struct PuntoMontaje));

		punto_Montaje->puntoMontajeBase = malloc ( string_length(configFile->puntoMontaje));
		punto_Montaje->puntoMontajeTables = malloc ( string_length(configFile->puntoMontaje) + string_length(MONTAJE_TABLES) ) ;
		punto_Montaje->puntoMontajeBloques = malloc ( string_length(configFile->puntoMontaje) + string_length(MONTAJE_BLOQUES) ) ;
		punto_Montaje->puntoMontajeMetadata = malloc (string_length(configFile->puntoMontaje) + string_length(MONTAJE_METADATA) ) ;



		strcpy(punto_Montaje->puntoMontajeBase,configFile->puntoMontaje);
		//printf("puntoMontaje %s\n",configFile->puntoMontaje);

//	//	creo punto de montajede Tablas
		strcpy(punto_Montaje->puntoMontajeTables,punto_Montaje->puntoMontajeBase);
		strcat(punto_Montaje->puntoMontajeTables,MONTAJE_TABLES);
		//mkdir(punto_Montaje->puntoMontajeTables,S_IRWXU);
//
//		//creo puntode montaje de Bloques
		strcpy(punto_Montaje->puntoMontajeBloques,punto_Montaje->puntoMontajeBase);
		strcat(punto_Montaje->puntoMontajeBloques,MONTAJE_BLOQUES);
//		mkdir(punto_Montaje->puntoMontajeBloques,S_IRWXU);
//		crearBloques();
//
//		//creo puntode montaje de Metadata
		strcpy(punto_Montaje->puntoMontajeMetadata,punto_Montaje->puntoMontajeBase);
		strcat(punto_Montaje->puntoMontajeMetadata,MONTAJE_METADATA);
//		mkdir(punto_Montaje->puntoMontajeMetadata,S_IRWXU);

//		printf("puntoMontajeMetadata: %s\n",punto_Montaje->puntoMontajeMetadata);
//		printf("puntoMontajeBloques: %s\n",punto_Montaje->puntoMontajeBloques);
//		printf("puntoMontajeTables: %s\n",punto_Montaje->puntoMontajeTables);
//		printf("puntoMontajeBase: %s\n",punto_Montaje->puntoMontajeBase);

}




/*********************************************/



void load_config_file(char *ruta) {

	//configFile = reservarMemoria(sizeof(archivoConfigLFS));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "LFS: Leyendo Archivo de Configuracion..");



		if (config_has_property(config, "PUERTO_ESCUCHA")) {
			configFile->puertoEscucha = config_get_int_value(config,
							"PUERTO_ESCUCHA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_ESCUCHA a la estrcutra de LFS. Valor: %d",configFile->puertoEscucha);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO de LFS con el parametro PUERTO_ESCUCHA");

		}

		if (config_has_property(config, "PUNTO_MONTAJE")) {
			configFile->puntoMontaje = strdup(
									config_get_string_value(config, "PUNTO_MONTAJE"));
							log_info(logger,
									"Se encontró y cargó el contenido de PUNTO_MONTAJE a la estructura de LFS. Valor: %s",configFile->puntoMontaje);

						} else {
							log_error(logger,
									"El archivo de configuracion no contiene el PUNTO_MONTAJE con el parametro PUNTO_MONTAJE");
		}

		if (config_has_property(config, "RETARDO")) {
			configFile->retardo = config_get_int_value(config,
							"RETARDO");
					log_info(logger,
							"Se encontró y cargó el contido del RETARDO a la estrcutra de LFS. Valor: %d",configFile->retardo);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el RETARDO de LFS con el parametro RETARDO");

		}

		if (config_has_property(config, "TAMAÑO_VALUE")) {
			configFile->tamanio_value = config_get_int_value(config,
								"TAMAÑO_VALUE");
						log_info(logger,
								"Se encontró y cargó el contido del TAMAÑO_VALUE a la estrcutra de LFS. Valor: %d",configFile->tamanio_value);
					} else {

						log_error(logger,
								"El archivo de configuracion no contiene el TAMAÑO_VALUE de LFS con el parametro TAMAÑO_VALUE");

		}

		if (config_has_property(config, "TIEMPO_DUMP")) {
			configFile->tiempo_dump = config_get_int_value(config,
										"TIEMPO_DUMP");
								log_info(logger,
										"Se encontró y cargó el contido del TIEMPO_DUMP a la estrcutra de LFS. Valor: %d",configFile->tiempo_dump);
							} else {

								log_error(logger,
										"El archivo de configuracion no contiene el TIEMPO_DUMP de LFS con el parametro TIEMPO_DUMP");

		}



		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"El archivo de configuración ha sido leído correctamente");
		//printf("El archivo de configuración ha sido leído correctamente\n");

		log_info(logger,
									"el valor de puerto escucha es: %d",configFile->puertoEscucha);


	}
}

void actualizar_file_config()
{

	char buffer[BUF_LEN];
	char* directorio = RUTA_CONFIG_CLIENTE;
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

					int retardo_anterior=configFile->retardo;
					int tiempo_dump_anterior=configFile->tiempo_dump;


					load_config_file(RUTA_CONFIG_CLIENTE);



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
