#include "principalesMem.h"
#include "VariablesMUSE.h"

void CrearLoggerMemoria() {
	char * archivoLog = strdup("MEM_LOG.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
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

void IniciarEstructuras()
{
	tabla_segmentos = list_create();
	tabla_gossiping = list_create();

	int sizeof_memoria = configFile->MEMORY_SIZE;
	memoria = reservarMemoria(sizeof_memoria);
	memset(memoria, '\0', sizeof_memoria);

	//El tamaño de la pagina es igual a timestamp (unsigned long 4 bytes) + key( int 4 bytes) + value()

	tamanioPagina = UNSIGNED_LONG_LONG + INT + tamanioValue;
	cantidad_de_marcos = sizeof_memoria / tamanioPagina;

	bitmap = reservarMemoria(sizeof(bitmap_t) * cantidad_de_marcos);
	int i = 0;
	for(; i < cantidad_de_marcos; i++){
		bitmap[i].modificado = 0;
		bitmap[i].ocupado = 0;
		bitmap[i].uso = 0;
		bitmap[i].timestamp = 0;
	}


	pthread_mutex_init(&mutex_socket_lfs, NULL);

}


int GestionarSolicitudes() //int esperar_y_AtenderScripts()
{
	fdEscucha = nuevoSocket();
	asociarSocket(fdEscucha, configFile->LISTEN_PORT);
	escucharSocket(fdEscucha, CONEXIONES_PERMITIDAS);


	fd_set setAux;
	fd_set setMaestro;
	int maxFD,i,socket_nuevo;

	FD_ZERO(&setMaestro); 	// borra los conjuntos maestro y temporal
	FD_ZERO(&setAux);

	maxFD = fdEscucha; //Llevo control del FD maximo de los sockets
	FD_SET(fdEscucha, &setMaestro); //agrego el FD del socketEscucha al setMaestro
	// Bucle principal:
	while(TRUE)
	{
		setAux = setMaestro;

		if (select((maxFD + 1), &setAux, NULL, NULL, NULL ) == -1) {
			//printf("Error en la escucha de Kernel\n");
			//log_error(logger,"Error en la escucha de Kernel\n" );
			return EXIT_FAILURE;
		}

		//Recorro las conexiones en busca de interacciones nuevas
		for (i = 0; i <= maxFD; i++)
		{
			if (FD_ISSET(i,&setAux))
			{
				// Me fijo en el set de descriptores a ver cual respondió
				if (i == fdEscucha)
				{
					//Tengo un nuevo hilo de kernel queriendose conectar
					//Esta funcion acepta una nueva conexion del kernel
					//y agrega un nuevo nodo a la lista de scripts con el nuevo socket
					log_info(logger,"llego un kernel");
					socket_nuevo = aceptarConexionEntrante();
					if (socket_nuevo == -1)
						return EXIT_FAILURE;
					FD_SET(socket_nuevo, &setMaestro); //agrego el nuevo socket al setMaestro
					if (socket_nuevo > maxFD)
						maxFD = socket_nuevo;

				}
				else
				{
					// Hay actividad nueva en algún hilo de kernel
					int *head=malloc(INT);
					char* query = aplicar_protocolo_recibir(i,head);

					//printf("valor de head: %i\n",*head);
					log_info(logger,"valor de head: %i",*head);
					//printf("query1: %s\n",query);
					log_info(logger,"query1: %s\n",query);
					atender_pedido(i, *head, query);

					FD_CLR(i, &setMaestro); // borra el file descriptor del set
					cerrarSocket(i); // cierra el file descriptor
				}
			}
		}


	}
}

void atender_pedido(int Socket, int head, char* query)
{
	switch(head)
	{
		case	MUSE_INIT: muse_init();
				break;
		case	MUSE_ALLOC: muse_alloc();
				break;
		case	MUSE_CLOSE: muse_close();
				break;
		case	MUSE_FREE:	muse_free();
				break;
		case	MUSE_GET:	muse_free();
				break;
		case	MUSE_CPY:	muse_cpy();
				break;
		case	MUSE_MAP:	muse_map();
				break;
		case	MUSE_SYNC:	muse_sync();
				break;
		case	MUSE_UNMAP:	muse_unmap();
				break;
		default: break;
	}
}

int muse_init(int id, char* ip, int puerto)
{
	IniciarEstructuras();
	LeerArchConfigMUSE();



}

void muse_close()
{

}

uint32_t muse_alloc(uint32_t tam)
{

	AgregarSegmentoTabla();
}

void muse_free(uint32_t dir)
{

}

int muse_get(void* dst, uint32_t src, size_t n)
{

}

int muse_cpy(uint32_t dst, void* src, int n)
{

}

uint32_t muse_map(char *path, size_t length, int flags)
{

}

int muse_sync(uint32_t addr, size_t len)
{

}

int muse_unmap(uint32_t dir)
{

}

int LeerArchConfigMUSE(char *ruta) {
	configFile = malloc(sizeof(archConfigMUSE));
	t_config *config;
	config = config_create(ruta);
	if (config != NULL) {
		configFile->LISTEN_PORT = strdup(config_get_string_value(config, "LISTEN_PORT"));
		configFile->MEMORY_SIZE = 500;//config_get_int_value(config, "MEMORY_SIZE");
		configFile->PAGE_SIZE = strdup(config_get_string_value(config, "PAGE_SIZE"));
		configFile->SWAP = config_get_int_value(config,"SWAP");
		config_destroy(config);
	}
	else return ERROR;
}
