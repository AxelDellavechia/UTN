#include "funcionesFuse.h"

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


/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 */

			/*
void *sac_init(struct fuse_conn_info *conn) {

	return FUSE_EXTRAS;
}
*/

int sac_getattr(const char *path, struct stat *stbuf ) {

	int head;

	memset(stbuf, 0, sizeof(struct stat));

	//Si path es igual a "/" nos estan pidiendo los atributos del punto de montaje

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return 0;
	}

	/*
	char * pathEnviar ;

	if( string_starts_with(* path,"/.") ) return -ENOENT;

	if ( string_contains( * path,"/.directory") ) {
		log_info(logger,"sac_getattr va a mandar cosas que no debe");
		int tam = string_length(path) - string_length("/.directory") ;
		pathEnviar = strdup( string_substring_until(path,tam) );
	}


	pathEnviar = strdup(path);
	*/

	GPath* fo =malloc(sizeof(GPath));

	fo->path = calloc(string_length(path)+1,sizeof(char)) ;
	strcpy((char*) fo->path, path);
	fo->path_tam = string_length(fo->path);

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_getattr le envio a Sac-Server la consulta por el path: %s",fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, GETATTR, fo);
	//pthread_mutex_unlock(&semaforo);

	void * mensajeRecibido =  aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_getattr recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		//free(pathEnviar);
		free(fo);
		return -ENOENT;
	}


	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_getattr recibio de Sac-Server: NULL");
		free(fo->path);
		//free(pathEnviar);
		free(fo);
		return -ENOENT;
	}

	GAttr* attr= (GAttr*) mensajeRecibido;

	log_info(logger,"sac_getattr recibio de Sac-Server -> File Size: %d , Tipo Archivo: %d , Modificación: %llu",attr->file_size,attr->state,attr->updated);


		if (attr->state == 1) stbuf->st_mode = S_IFREG	| 777 ;
		if (attr->state == 2) stbuf->st_mode = S_IFDIR	| 777 ;
		stbuf->st_nlink = 2;
		stbuf->st_size = attr->file_size;


	free(fo->path);
	//free(pathEnviar);
	free(fo);
	free(mensajeRecibido);
//	free(attr);
    return 0;
}


/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 */

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {

	(void) offset;
	(void) fi;

	int head ;

	/*
	char * pathEnviar ;
	if ( string_ends_with(path,"/.directory") ) {
		int tam = string_length(path) - string_length("/.directory") ;
		pathEnviar = strdup( string_substring_until(path,tam) );
	}


	pathEnviar = strdup(path);
	*/

	GPath* fo = malloc(sizeof(GPath));

	fo->path = calloc(string_length(path),sizeof(char)); strcpy((char*) fo->path, path);
	fo->path_tam = string_length(fo->path);

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_readdir le envio a Sac-Server la consulta por el path: %s",fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, READDIR, fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	void * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO ) {

		log_info(logger,"sac_readdir recibio de Sac-Server: FIN DE PROTOCOLO");

		//free(pathEnviar);
		free(fo->path);
		free(fo);
		return -ENOENT;
	}

	if(mensajeRecibido == NULL) {
		filler( buf, ".", NULL, 0 );  // Current Directory
		filler( buf, "..", NULL, 0 ); // Parent Directory

		//free(pathEnviar);
		free(fo->path);
		free(fo);

		return 0;
	}

	char * elMensaje = strdup(mensajeRecibido);

	free(mensajeRecibido);

	int elementos = cantFileName(elMensaje) ;

	char ** elementoSeparado = string_n_split(elMensaje,elementos,"|") ;



	filler( buf, ".", NULL, 0 );  // Current Directory
	filler( buf, "..", NULL, 0 ); // Parent Directory

	for ( int contador = 0 ; contador < elementos -1 ; contador ++){
			log_info(logger,"sac_readdir recibio de Sac-Server: %s", elementoSeparado[contador]);
			filler(buf, elementoSeparado[contador] , NULL, 0);
	}

	for ( int contador = 0 ; contador < elementos ; contador ++){
				free(elementoSeparado[contador]);
	}

	free(elementoSeparado) ;
	free(elMensaje);
	//free(pathEnviar);
	free(fo->path);
	free(fo);

    return 0;
}


int sac_truncate (const char* path, off_t length){

		int head ;

		return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para tratar de abrir un archivo
 */

int sac_open(const char *path, struct fuse_file_info *fi) {

	int head ;

	GOpen* archivo = malloc(sizeof(GOpen));

	//fi->keep_cache = 0 ;

	//fi->writepage = 1 ;

	archivo->path_tam = string_length(path);

	archivo->path = strdup(path);

	if ((fi->flags & 3) != O_RDONLY) return -EACCES;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_open le envio a Sac-Server la consulta por el path: %s con los flags -> create:%d , ensure:%d , trunc: %d",path,archivo->create,archivo->ensure,archivo->trunc);


	aplicar_protocolo_enviar(fdSacServer, OPEN , archivo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_open recibio de Sac-Server: NULL");
		free(archivo->path);
		free(archivo);
		return -ENOENT;
	}

	log_info(logger,"sac_open recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(archivo->path);
		free(archivo);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(archivo->path);
		free(archivo);

		return 0;
	}

}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener el contenido de un archivo
 */

int sac_release (const char * path, struct fuse_file_info * fildes){
	return 0;
}

int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {

	size_t len;
	(void) fi;

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_read le envio a Sac-Server la consulta por el path: %s",path);

	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));
	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);
	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, READ , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	char * mensajeRecibido = strdup( aplicar_protocolo_recibir(fdSacServer,&head) ) ;
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_read recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(pathEnviar);
		free(fo);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_read recibio de Sac-Server: NULL");
		free(fo->path);
		free(pathEnviar);
		free(fo);
		return -ENOENT;
	}

	log_info(logger,"sac_read recibio Sac-Server lo siguiente: %s",mensajeRecibido);

	len = strlen(mensajeRecibido);

	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, mensajeRecibido + offset, size);
		free(fo->path);
		free(pathEnviar);
		free(fo);
		free(mensajeRecibido);
	} else
		size = 0;

	free(fo->path);
	free(pathEnviar);
	free(fo);
	free(mensajeRecibido);

	return size;

}

int sac_create(const char *path, mode_t mode, struct fuse_file_info *fi) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_create le envio a Sac-Server la consulta por el path: %s",path);

	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));
	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);
	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, CREATE , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_create recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_create recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_create recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}

int sac_mkdir(const char *path, mode_t mode) {

	int head ;
	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));
	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);
	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, MKDIR , fo);
	pthread_mutex_unlock(&semaforo);

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_mkdir le envio a Sac-Server la consulta por el path: %s",path);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_mkdir recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_mkdir recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_mkdir recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}


int sac_mknod( const char *path, mode_t mode, dev_t rdev) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_mknod le envio a Sac-Server la consulta por el path: %s",path);

	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));
	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);
	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, MKNOD , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_mknod recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_mknod recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_mknod recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}


int sac_write( const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_write le envio a Sac-Server la consulta por el path: %s",path);


	if (buf == NULL || strcmp(buf,"") == 0 ) return -ENOENT ;

	pthread_mutex_lock(&semaforo);

	strcat(path,"|");

	strcat(path,buf);

	log_info(logger,"sac_write le va a enviar a Sac-Server lo siguiente: buff tiene un tamanio de: %d",string_length(path));

	//char * pathEnviar ;
	//pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));

	fo->path = calloc(string_length(path)+1,sizeof(char));
	strcpy((char*) fo->path, path);
	fo->path_tam = string_length(fo->path);



	aplicar_protocolo_enviar(fdSacServer, WRITE , fo);

	int * respuesta = aplicar_protocolo_recibir(fdSacServer,& head);

	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_write recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		//free(mensajeRecibido);
		return -ENOENT;
	}

	if( respuesta == NULL) {
		log_info(logger,"sac_write recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		//free(mensajeRecibido);
		return -ENOENT;
	}

	log_info(logger,"sac_write recibio Sac-Server lo siguiente: %d",* respuesta);

	if (* respuesta < 0) {
		free(fo->path);
		free(fo);
		//free(mensajeRecibido);
		capturarErroresFS( * respuesta);
	} else {
		free(fo->path);
		free(fo);
		//free(mensajeRecibido);
		return * respuesta;
	}
}

int sac_unlink( const char *path) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_unlink le envio a Sac-Server la consulta por el path: %s",path);

	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));
	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);
	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, UNLINK , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_unlink recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_unlink recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_unlink recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}

int sac_rmdir( const char *path) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_rmdir le envio a Sac-Server la consulta por el path: %s",path);

	char * pathEnviar ;
	pathEnviar = strdup(path);

	GPath* fo =malloc(sizeof(GPath));

	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);

	fo->path_tam = string_length(fo->path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, RMDIR , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_rmdir recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_rmdir recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_rmdir recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}


int sac_utimes (const char *path, struct timeval tvp[2]){

	/*
	int head ;

	sac_utime_file * tiempoActual;

	unsigned long long tiempo  = ( ( (unsigned long long )tvp->tv_sec) * 1000 + ( ( unsigned long ) tvp->tv_usec ) / 1000 );
	tiempoActual->path = malloc(strlen(path));
	tiempoActual->path = strdup(path) ;
	tiempoActual->path_tam = string_length(path)  ;
	tiempoActual->utime = tiempo ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_utimes le envio a Sac-Server la consulta por el path: %s",path);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, UTIMES , tiempoActual);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_utimes recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(tiempoActual->path);
		free(tiempoActual);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_utimes recibio de Sac-Server: NULL");
		free(tiempoActual->path);
		free(tiempoActual);
		return -ENOENT;
	}

	log_info(logger,"sac_utimes recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(tiempoActual->path);
		free(tiempoActual);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(tiempoActual->path);
		free(tiempoActual);
		return 0;
	}
	*/

	return 0 ;
}


int sac_rename(const char* path,const char* newName) {

	int head ;

	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_rename le envio a Sac-Server el cambio de nombre del path: %s por el siguiente: %s ",path,newName);

	char * pathEnviar ;

	pathEnviar = strdup(path);

	GPathRename* fo =malloc(sizeof(GPathRename));

	fo->path = calloc(string_length(pathEnviar),sizeof(char)); strcpy((char*) fo->path, pathEnviar);

	fo->path_tam = string_length(fo->path);
	fo->pathNew = strdup(newName);
	fo->pathNew_tam = string_length(fo->path_tam);

	pthread_mutex_lock(&semaforo);
	aplicar_protocolo_enviar(fdSacServer, RENAME , fo);
	pthread_mutex_unlock(&semaforo);

	pthread_mutex_lock(&semaforo);
	int * mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	pthread_mutex_unlock(&semaforo);

	if ( head == FIN_DEL_PROTOCOLO  ) {
		log_info(logger,"sac_rename recibio de Sac-Server --> FIN_DEL_PROTOCOLO " ) ;
		free(fo->path);
		free(fo->pathNew);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	if( mensajeRecibido == NULL) {
		log_info(logger,"sac_rename recibio de Sac-Server: NULL");
		free(fo->path);
		free(fo->pathNew);
		free(fo);
		free(pathEnviar);
		return -ENOENT;
	}

	log_info(logger,"sac_rename recibio Sac-Server lo siguiente: %d", * mensajeRecibido);

	if (* mensajeRecibido < 0) {
		free(fo->path);
		free(fo->pathNew);
		free(fo);
		free(pathEnviar);
		capturarErroresFS( * mensajeRecibido);
	} else {
		free(fo->path);
		free(fo->pathNew);
		free(fo);
		free(pathEnviar);
		return 0;
	}
}


/*
int sac_flush( const char *path, struct fuse_file_info *fi) {

	int head ;

	log_info(logger,"sac_flush le envio a Sac-Server la consulta por el path: %s",path);
		memcpy(buffer+desplazamiento,pathRename->pathNew,pathRename->pathNew_tam);
		desplazamiento += pathRename->pathNew_tam;
//	aplicar_protocolo_enviar(fdSacServer, FLUSH , path);

//	int mensajeRecibido = aplicar_protocolo_recibir(fdSacServer,& head);
	int mensajeRecibido= 1;
	log_info(logger,"sac_flush recibio Sac-Server lo siguiente: %d",mensajeRecibido);

	if (mensajeRecibido < 0) capturarErroresFS(mensajeRecibido);

	return mensajeRecibido;

}
*/

int sac_chmod(const char *path, mode_t mode)
{
    int respuesta;
	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_chmod le envio a Sac-Server la consulta por el path: %s",path);
    respuesta = chmod(path, mode);
    log_info(logger,"sac_chmod recibio Sac-Server lo siguiente: %d", respuesta);
    if(respuesta == -1)
        return -ENOENT;
    return 0;
}


int sac_chown(const char *path, uid_t uid, gid_t gid)

{
    int respuesta;
	log_info(logger,"-------------------------------------------------------------");
	log_info(logger,"sac_chown le envio a Sac-Server la consulta por el path: %s",path);
    respuesta = chown(path,uid,gid);
    log_info(logger,"sac_chown recibio Sac-Server lo siguiente: %d", respuesta);
    if(respuesta == -1)
        return -ENOENT;
    return 0;
}
