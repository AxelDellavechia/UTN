#include "Generales.h"
#include "funcionesFuse.h"

void inicializar_semaforos(){

	pthread_mutex_init(&semaforo, NULL);

}

int capturarErroresFS ( int error ) {
	log_info(logger,"Validando el error devuelto por el Sac-Server:");
	switch (error) {
				case  -1 :
					log_info(logger,"NO_EXISTE_PATH");
					return -EIO ; // ERROR -1 // NO_EXISTE_PATH -1
				break ;
				case  -2 :
					log_info(logger,"NO_POSEE_PADRE");
					return -EPIPE ; // NO_POSEE_PADRE -2
				break ;
				case  -3 :
					log_info(logger,"NOMBRE_DEMASIADO_LARGO");
					return -EFBIG; // NOMBRE_DEMASIADO_LARGO -3
				break ;
				case  -4 :
					log_info(logger,"NO_HAY_MAS_NODOS_LIBRES");
					return -ENFILE ; // NO_HAY_MAS_NODOS_LIBRES -4
				break ;
				case  -5 :
					log_info(logger,"ARCHIVO_YA_EXISTE");
					return -EEXIST; // ARCHIVO_YA_EXISTE -5
				break ;
				case  -6 :
					log_info(logger,"PATH_VACIO");
					return -ENOENT; // PATH_VACIO -6
				case  -7 :
					log_info(logger,"PATH_NO_VACIO");
					return -EPERM; // PATH_NO_VACIO -6
				break;

	}
}

unsigned long long obtener_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = ((unsigned long long)tv.tv_sec*1e3) + ((unsigned long long)tv.tv_usec/1000);
	return timestamp;
}

void capturarError(int signal) {
	if (signal == SIGINT ) {
		cerrarSocket(fdSacServer);
		//free(param);
		free(configSacCli);
		exit(1);
	}
}

void crearLogger() {
	char * archivoLog = strdup("SacCli.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}

void leerArchivoDeConfiguracion(char *ruta,t_log * logger) {

	t_config *config;
	config = config_create(ruta);
	if (config != NULL) {
		log_info(logger, "Leyendo Archivo de Configuracion..");

		if (config_has_property(config, "IP_SAC_SERVER")) {
			configSacCli->ipSacServer = strdup(
											config_get_string_value(config, "IP_SAC_SERVER"));
									log_info(logger,
											"Se encontró y cargó el contenido de IP_SAC_SERVER a la estructura. Valor: %s",configSacCli->ipSacServer);

		} else {
									log_error(logger,
											"El archivo de configuracion no contiene el IP_SAC_SERVER");
				}

		if (config_has_property(config, "PUERTO_SAC_SERVER")) {
			configSacCli->puertoSacServer = config_get_int_value(config,
								"PUERTO_SAC_SERVER");
						log_info(logger,
								"Se encontró y cargó el contido del PUERTO_SAC_SERVER. Valor: %d",configSacCli->puertoSacServer);
					} else {

						log_error(logger,
								"El archivo de configuracion no contiene el PUERTO_SAC_SERVER");

			}

		if (config_has_property(config, "PUERTO_SAC_CLI")) {
					configSacCli->puertoEscucha = config_get_int_value(config,
										"PUERTO_SAC_CLI");
								log_info(logger,
										"Se encontró y cargó el contido del PUERTO_SAC_CLI. Valor: %d",configSacCli->puertoEscucha);
							} else {

								log_error(logger,
										"El archivo de configuracion no contiene el PUERTO_SAC_CLI");

					}
/*
		if (config_has_property(config, "PUNTO_MONTAJE")) {
					configSacCli->puntoMontaje = strdup(
													config_get_string_value(config, "PUNTO_MONTAJE"));
											log_info(logger,
													"Se encontró y cargó el contenido de PUNTO_MONTAJE a la estructura. Valor: %s",configSacCli->puntoMontaje);

				} else {
											log_error(logger,
													"El archivo de configuracion no contiene el PUNTO_MONTAJE");
		}*/

	}
	free(config);
}

void* reservarMemoria(int size) {

		void *puntero = malloc(size);
		if(puntero == NULL) {
			fprintf(stderr, "Error al reservar %d bytes de memoria", size);
			exit(ERROR);
		}
		return puntero;
}

int cantFileName(char* path)
{
	int tamanio = 1;
	char *ptrPath = path;

	tamanio =string_length(path);
	if (tamanio < 1)
	{
		return 0;
	}


	int acum=1;
	for(int i=0; i< string_length(path); i++)
	{

	//if(*(ptrPath)=='\\')
	if(*(ptrPath)== '|')
	{
		   acum++;
	 }
	ptrPath = ptrPath + 1;
	}
	return acum;
}

/*

void pathCompleto(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, FUSE_EXTRAS->rootdir);
    strncat(fpath, path, PATH_MAX);
}

int consolaFuse(){

	// printf("Fuse se estará ejecutando en background y aquí visualizará la informacion de su FS");

	return fuse_main(args.argc, args.argv, &sacCli_oper,NULL);
}


void consola() {

	printf("Hola! este es el modulo SAC-Cli una interfaz de comunicación con SAC-Server y "
			"se configuro con los siguientes datos:");
	printf("--------------------------------------------------------");
	printf("		IP SAC-Server: %s",configSacCli->ipSacServer);
	printf("		Puerto SAC-Server: %d",configSacCli->puertoSacServer);
	printf("		Puerto SAC-Cli: %d",configSacCli->puertoEscucha);
	printf("--------------------------------------------------------");
	printf ("Para finalizar módulo ingrese \"SALIR\"");

	size_t buffer_size = 100;

	char* comando = (char *) calloc(1, buffer_size);

	while (!string_equals_ignore_case(comando, "salir")) {
		printf(">");
		int bytes_read = getline(&comando, &buffer_size, stdin);
		if (bytes_read == -1) {
			exit(0);
		}
		if (bytes_read == 1) {
			continue;
		}

	}

	free(comando);
}

char *my_itoa(int num, char *str)
{
        if(str == NULL)
        {
                return NULL;
        }
        sprintf(str, "%d", num);
        return str;
}

void fullPath(char* ruta) {
	fpath = (char *) malloc( 1 + strlen(configSacCli->puntoMontaje) + strlen(ruta) );
	strcpy(fpath,configSacCli->puntoMontaje);
	string_append(&fpath,ruta);
	log_info(fuse_extras->logger,"la ruta completa es: %s", fpath ) ;
}


char * generarPaqueteFuncion(char * operacion , char * mensaje ){

	int tamanio = 1 + strlen("<") + strlen(operacion) + strlen("|") + strlen(mensaje) + strlen(">")  ;

	char c_tam[tamanio];

	my_itoa(tamanio,c_tam);

	char * mensajeCompleto = (char *) malloc( strlen(c_tam) + tamanio ) ;

	strcpy(mensajeCompleto,"<");
	strcat(mensajeCompleto,c_tam);
	strcat(mensajeCompleto,"|");
	strcat(mensajeCompleto,operacion);
	strcat(mensajeCompleto,mensaje);
	strcat(mensajeCompleto,">");

	return mensajeCompleto ;
}

char * parse( char * buffer ) {

	int cantidadParametros = 1 ; int recorrerArray= 0 ; int indiceCampo = 0 ; int tamBuffer ;

	if ( buffer != NULL ){

		tamBuffer = string_length(buffer) ;

		char * aux = (char * ) malloc (string_length(buffer));

		for(recorrerArray ; recorrerArray < tamBuffer ; recorrerArray ++ ){

				if( buffer[recorrerArray] == '|' ){
					cantidadParametros++ ;
				}
				if(buffer[recorrerArray] != '<' && buffer[recorrerArray] != '>') {
					aux[indiceCampo] = buffer[recorrerArray] ;
					indiceCampo++;
				}

			}

		return string_n_split(aux , cantidadParametros,"|");
	}  else {
		return buffer;
	}
}

*/

