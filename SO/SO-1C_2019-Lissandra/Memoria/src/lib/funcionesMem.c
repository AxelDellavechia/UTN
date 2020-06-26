
#include "funcionesMem.h"

int validar_servidor(char *id) {
	if(strcmp(id, "F") == 0 || strcmp(id, "M") == 0 ) {
		log_info(logger,"Servidor aceptado.\n");
		return TRUE;
	} else {
		printf("Servidor rechazado.\n");
		return FALSE;
	}
}


int aceptarConexionEntranteDeKernel()
{
	int new_fd = aceptarConexionSocket(fdEscuchaKernel);

	int ret_handshake = handshake_servidor(new_fd, "M");

	if(ret_handshake == FALSE){ // Falló el handshake
		log_error(logger, "Conexión inicial fallida el script fd #%d.", new_fd);
		cerrarSocket(new_fd);
		return ERROR;
	}

	// Se conectó un nuevo hilo del kernel para la ejecucion de un script
	query_t* nuevoScript = malloc(sizeof(query_t));
	nuevoScript->id_sript = new_fd - fdEscuchaKernel;
	nuevoScript->fd_script = new_fd;
	log_info(logger,"Un hilo nuevo de Kernel se ha conectado para la ejecución de un script", nuevoScript->id_sript);

	// Agrego al Script a la lista de Scripts:
	list_add(lista_querys, nuevoScript);
	log_info(logger,"El script #%i se ha conectado.", nuevoScript->id_sript);

	//Revisar si le tengo que envíar algo más al script.

	return new_fd;

}

void add_nuevo_script(char* consulta ){

	query_t* nueva_query = malloc(sizeof(query_t));
	nueva_query->query = malloc(TAM_MAX_QUERY);
	memset(nueva_query->query,'\0',TAM_MAX_QUERY);
	memcpy(nueva_query->query,consulta,TAM_MAX_QUERY);

	list_add(lista_querys, nueva_query);



}

void recorrer_lista_querys_y_atenderlos(){
	int i;
	int bytes_enviados;
	printf("Estoy recorriendo la lista de querys");
	for (i = 0; i < list_size(lista_querys); i++){
		query_t * una_query = (query_t*) list_get(lista_querys, i);
		printf("query 3: %s\n",una_query->query);
		bytes_enviados = aplicar_protocolo_enviar(fdLfs,DESCRIBE,una_query->query);

	}

}
int obtenerSocketMaximoInicial(){
	//Por ahora el maximo es solo el socket de escucha a Kernel
	return fdEscuchaKernel;
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

int validar_cliente(char *id){
	if(strcmp(id, "K")== 0 || strcmp(id, "M")== 0 ) {
				log_info(logger,"Cliente aceptado.");
				return TRUE;
			}
	else {
				printf("Cliente rechazado.\n");
				return FALSE;
			}
}

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



void remueve_salto_de_linea(char* salida, char* texto) {
	strncpy(salida, texto, strlen(texto) - 1);
}
char* comando_preparado(char* comando) {
	char* comando_listo = calloc(1, strlen(comando));
	remueve_salto_de_linea(comando_listo, comando);
	return comando_listo;
}


char* ull_to_string(unsigned long long valor){
	char buffer[100] = {'\0'};
	int tamanio = sprintf(buffer,"%llu",valor);
	char* valor_char = malloc(tamanio + 1);
	memset(valor_char,'0',tamanio+1);
	strcpy(valor_char,buffer);

	return valor_char;
}
char* crear_mensaje_journal(int nro_marco, char* nombre_tabla){


	int tam_nom_tabla = strlen(nombre_tabla);


	pagina_t* pag = get_marco(nro_marco);
	char* timestamp = ull_to_string(pag->timestamp);

	printf("timestamp  : %s\n",timestamp);

	int tam_total = tam_nom_tabla + CHAR +  INT + CHAR + tamanioValue + CHAR + UNSIGNED_LONG_LONG + CHAR;
	char* query = malloc(tam_total);
	memset(query,'\0',tam_total);


	string_append(&query,nombre_tabla);
	string_append(&query,"|");
	string_append(&query,string_itoa(pag->key));
	string_append(&query,"|");
	string_append(&query,pag->value);
	string_append(&query,"|");
	string_append(&query,timestamp);
	string_append(&query,"\0");
	printf("query de journal: %s\n",query);

	free(timestamp);
	free(pag->value);
	free(pag);
	return query;


}

int ejecutar_journal(){
	int i,k;
	printf("init ejecutar_journal\n");
	for(i=0;i<list_size(tabla_segmentos);i++){
		tablaSegmentos_t* tabla = (tablaSegmentos_t*) list_get(tabla_segmentos,i);
		for(k=0;k<list_size(tabla->tablaPaginas);k++){
			tablaPagina_t* t_pag = (tablaPagina_t*) list_get(tabla->tablaPaginas,k);
			if(t_pag->bit_modificado == 1){
				char* reg = crear_mensaje_journal(t_pag->nro_marco,tabla->nombre_tabla );
				int head;

				pthread_mutex_lock(&mutex_socket_lfs);

				int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,JOURNAL,reg);
				char* resultado = aplicar_protocolo_recibir(fdLfs,&head);


				pthread_mutex_unlock(&mutex_socket_lfs);

				//printf("bytes_enviados: %i\n",bytes_enviados);
				if(strcmp(resultado, NO_EXISTE_LA_TABLA) == 0){
					log_error(logger," ERROR-JOURNAL-La tabla no existe en el File System.");
					printf(" ERROR-JOURNAL-La tabla no existe en el File System.\n");
				}
			}
		}
	}
	printf("fin ejecutar_journal\n");
	return 1;

}

void limpiar_bitmap(){
	int i = 0;
	for(; i < cantidad_de_marcos; i++){
		bitmap[i].modificado = 0;
		bitmap[i].ocupado = 0;
		bitmap[i].timestamp = 0;
	}
}

void limpiar_tabla_segmentos_y_paginas(){

	int i;
	for(i=0;i<list_size(tabla_segmentos);i++){
		tablaSegmentos_t* seg = list_get(tabla_segmentos,i);
		int k;
		for(k=0;k<list_size(seg->tablaPaginas);k++){
			tablaPagina_t* pag = list_get(seg->tablaPaginas,k);
			free(pag);
		}
		seg->tablaPaginas = NULL;

		free(seg);
	}

	list_clean(tabla_segmentos);

}
void limpiar_toda_la_memoria(){

	limpiar_tabla_segmentos_y_paginas();

	limpiar_bitmap();

	int sizeof_memoria = configFile->tamanioMem;
	memset(memoria, '\0', sizeof_memoria);
}
void ejecutar_comando_journal_kernel(int socket_kernel, char* query){
	ejecutar_journal();
	limpiar_toda_la_memoria();
	aplicar_protocolo_enviar(socket_kernel,JOURNAL,SUCCESSFUL);

}
int  ejecutar_comando_journal(){

	ejecutar_journal();
	limpiar_toda_la_memoria();
	printf("JOURNAL termino exitosamente\n");
	return 1;
}

void atender_memoria_full_select(int socket_kernel,char* query){

	char** campos = string_n_split(query,3," ");
	char* nombre_tabla = campos[1];
	int key = atoi(campos[2]);

	int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,MEMORIA_FULL);
	//atender_memoria_full();
	int head;
	aplicar_protocolo_recibir(socket_kernel,&head);
	if(head == JOURNAL){
		ejecutar_journal();
	}

	//Consulto de nuevo  al lfs para tener el valor mas actualizado
	pthread_mutex_lock(&mutex_socket_lfs);
	int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
	char* result = aplicar_protocolo_recibir(fdLfs,&head);
	pthread_mutex_unlock(&mutex_socket_lfs);

	int rta = agregar_segmento_pagina(nombre_tabla,key,result,0);

	aplicar_protocolo_enviar(socket_kernel,JOURNAL,result);
	//atender_select_por_segunda_vez(socket_kernel);

	return;
}


void  ejecutar_comando_select_kernel(int socket_kernel, char* query){

	//printf("query1 : %s\n",query);
	char** campos = string_n_split(query,3," ");
	char* nombre_tabla = campos[1];
	int key = atoi(campos[2]);
	//printf("nombre_tabla: %s\n",nombre_tabla);
	//printf("key: %i\n", key);
	int index_segmento = existe_segmento(nombre_tabla);
	//printf("index_segmento: %i\n", index_segmento);
	if(index_segmento >= 0)
	{
		int  index_pagina = existe_key(key,index_segmento);
		if( index_pagina >= 0){//Existe la key
			pagina_t* pag = get_valor_de_key_en_memoria(index_pagina,index_segmento);
			//printf("pag->value: %s\n",pag->value);
			///////////////////**////////////////

			char* valor_final = malloc(tamanioValue + CHAR + UNSIGNED_LONG_LONG + CHAR);
			memset(valor_final,'\0',tamanioValue + CHAR + UNSIGNED_LONG_LONG + CHAR);
			char* time_aux = ull_to_string(pag->timestamp);

			string_append(&valor_final,pag->value);
			string_append(&valor_final," ");
			string_append(&valor_final,time_aux);

			free(time_aux);

			///////////////////**////////////////
			int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,valor_final);
			//printf("bytes_env: %i\n", bytes_env);
			free(pag->value);
			free(pag);
			free(valor_final);
		}else{

			int head;
			pthread_mutex_lock(&mutex_socket_lfs);
			int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
			char* result = aplicar_protocolo_recibir(fdLfs,&head);
			pthread_mutex_unlock(&mutex_socket_lfs);
			//printf("bytes_enviados: %i\n", bytes_enviados);
			//printf("result: %s\n",result);

			if(strcmp(result,NO_EXISTE_LA_KEY) == 0){
				//Enviar a kernel no existe la key
			}

			if(strcmp(result,NO_EXISTE_LA_TABLA) == 0){
				//Enviar a kernel no existe la tabla
			}

			int nro_pag = agregar_entrada_tabla_pagina(index_segmento, 0);

			if(nro_pag >= 0){
				agregar_pagina(result, key, nro_pag, index_segmento,0);
				int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,result);
				//printf("bytes_env: %i\n", bytes_env);


			}




			free(result);
		}

	 }else{

		//printf("query2 : %s\n",query);
		int head;
		pthread_mutex_lock(&mutex_socket_lfs);
		int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
		char* result = aplicar_protocolo_recibir(fdLfs,&head);
		pthread_mutex_unlock(&mutex_socket_lfs);
		//printf("bytes_enviados: %i\n", bytes_enviados);
		//printf("result: %s\n",result);

//		if(strcmp(result,NO_EXISTE_LA_KEY) == 0){
//			//Enviar a kernel no existe la key
//			printf("no existe la tabla\n");
//		}



		if(strcmp(result,NO_EXISTE_LA_KEY) == 0){
			//Enviar a kernel no existe la tabla
			printf("no existe la key\n");
			aplicar_protocolo_enviar(socket_kernel,SELECT, NO_EXISTE_LA_KEY );
		}else{

			int rta = agregar_segmento_pagina(nombre_tabla,key,result,0);

			if(rta >= 0){
				int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,result);
				//printf("bytes_enviados: %i\n", bytes_enviados);

			}

		}

		free(result);
	 }
}

//
//void  ejecutar_comando_select_kernel(int socket_kernel, char* query){
//
//	char** campos = string_n_split(query,3," ");
//	char* nombre_tabla = campos[1];
//	int key = atoi(campos[2]);
//	printf("nombre_tabla: %s\n",nombre_tabla);
//	printf("key: %i\n", key);
//	int index_segmento = existe_segmento(nombre_tabla);
//	printf("index_segmento: %i\n", index_segmento);
//	if(index_segmento >= 0)
//	{
//		int  index_pagina = existe_key(key,index_segmento);
//		if( index_pagina >= 0){//Existe la key
//			pagina_t* pag = get_valor_de_key_en_memoria(index_pagina,index_segmento);
//			printf("pag->value: %s\n",pag->value);
//			int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,pag->value);
//			printf("bytes_env: %i\n", bytes_env);
//			free(pag->value);
//			free(pag);
//		}else{
//			int head;
//			pthread_mutex_lock(&mutex_socket_lfs);
//			int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
//			char* result = aplicar_protocolo_recibir(fdLfs,&head);
//			pthread_mutex_unlock(&mutex_socket_lfs);
//			printf("bytes_enviados: %i\n", bytes_enviados);
//			printf("result: %s\n",result);
//			int nro_pag = agregar_entrada_tabla_pagina(index_segmento, 0);
//
//			if(nro_pag >= 0){
//				agregar_pagina(result, key, nro_pag, index_segmento);
//				int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,result);
//				printf("bytes_env: %i\n", bytes_env);
//
//
//			}
//
//			if(nro_pag == atoi(MEMORIA_FULL))
//				atender_memoria_full_select(socket_kernel);
//
//
//
//			free(result);
//		}
//
//	 }else{
//		int head;
//		pthread_mutex_lock(&mutex_socket_lfs);
//		int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
//		char* result = aplicar_protocolo_recibir(fdLfs,&head);
//		pthread_mutex_unlock(&mutex_socket_lfs);
//		printf("bytes_enviados: %i\n", bytes_enviados);
//		printf("result: %s\n",result);
//		int rta = agregar_segmento_pagina(nombre_tabla,key,result,0);
//		if(rta >= 0){
//			int bytes_env = aplicar_protocolo_enviar(socket_kernel,SELECT,result);
//			printf("bytes_enviados: %i\n", bytes_enviados);
//
//
//
//		}
//
//		if(rta == atoi(MEMORIA_FULL))
//			atender_memoria_full_select(socket_kernel);
//
//
//
//		free(result);
//	 }
//}

int  ejecutar_comando_select(char* nombre_tabla, int key, char* query)
{

	if((nombre_tabla != NULL) & (key != 0) )
	{
		int index_segmento = existe_segmento(nombre_tabla);
		if(index_segmento >= 0)
		{
			int  index_pagina = existe_key(key,index_segmento);
			if( index_pagina >= 0){//Existe la key
				pagina_t* pag = get_valor_de_key_en_memoria(index_pagina,index_segmento);
				printf("Timestamp: %llu\n",pag->timestamp);
				printf("Key: %i\n",pag->key);
				printf("Valor: %s\n", pag->value);
				free(pag->value);
				free(pag);
			}else{
				int head;
				pthread_mutex_lock(&mutex_socket_lfs);
				int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
				char* result = aplicar_protocolo_recibir(fdLfs,&head);
				pthread_mutex_unlock(&mutex_socket_lfs);
				//printf("bytes_enviados: %i\n", bytes_enviados);

				if(strcmp(result,NO_EXISTE_LA_KEY) == 0){
					printf("No existe la key en el LFS\n");
				}

				if(strcmp(result,NO_EXISTE_LA_TABLA) == 0){
					printf("No existe la tabla en el LFS\n");
				}
				printf("result: %s\n",result);
				int nro_pag = agregar_entrada_tabla_pagina(index_segmento, 0);

				if(nro_pag >= 0){
					agregar_pagina(result, key, nro_pag, index_segmento, 0);
				}






				free(result);
			}


		}else{
			int head;
			pthread_mutex_lock(&mutex_socket_lfs);
			int bytes_enviados = aplicar_protocolo_enviar(fdLfs,SELECT,query);
			char* result = aplicar_protocolo_recibir(fdLfs,&head);
			pthread_mutex_unlock(&mutex_socket_lfs);
			//printf("bytes_enviados: %i\n", bytes_enviados);
			printf("result: %s\n", result);

			if(strcmp(result,NO_EXISTE_LA_KEY) == 0){
				printf("No existe la key en el LFS \n");
			}

			if(strcmp(result,NO_EXISTE_LA_TABLA) == 0){
				printf("No existe la tabla en el LFS\n");
			}

			int rta = agregar_segmento_pagina(nombre_tabla,key,result,0);




			free(result);
		}
	}

//	if((nombre_tabla != NULL) & (key == 0) )
//	{
//
//		int index_segmento = existe_segmento(nombre_tabla);
//		if(index_segmento >= 0)
//		{
//			tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);
//			printf("******************EL NOMBRE DE LA TABLA ES: %s **************\n",segmento->nombre_tabla);
//
//			printf("La cantidad de paginas del segmento es: %i\n",segmento->cant_Paginas);
//			int j=0;
//			int cantidadDePaginas = list_size(segmento->tablaPaginas);
//			for(; j < cantidadDePaginas; j++)
//			{
//
//				tablaPagina_t* nodo_pagina = (tablaPagina_t*) list_get(segmento->tablaPaginas,j);
//				printf("****** DATOS DE LA PAGINA NRO %i  *********\n",nodo_pagina->nro_pagina);
//				printf("El bit modificado es: %i\n",nodo_pagina->bit_modificado);
//
//				printf("El nro marco  es: %i\n",nodo_pagina->nro_marco);
//
//
//				char* ptr =  memoria + (nodo_pagina->nro_marco * tamanioPagina);
//				pagina_t *pag = malloc(sizeof(pagina_t));
//
//				int desplazamiento = 0;
//				memcpy(&pag->timestamp, ptr + desplazamiento, UNSIGNED_LONG_LONG);
//				desplazamiento += UNSIGNED_LONG_LONG;
//				memcpy(&pag->key, ptr + desplazamiento, INT);
//				desplazamiento += INT;
//				pag->value = malloc(tamanioValue);
//				memset(pag->value,'\0',tamanioValue);
//				memcpy(pag->value, ptr + desplazamiento,tamanioValue );
//				printf("El timestamp es: %llu\n",pag->timestamp);
//				printf("La key es: %i\n",pag->key);
//				printf("el valor de value es: %s\n",pag->value);
//
//				free(pag);
//
//
//			}
//
//		}
//	}

	return EXIT_SUCCESS;

}



void imprimir_tabla_segmentos()
{
	int i = 0;
	for(; i < list_size(tabla_segmentos); i++) {
		tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,i);
		printf("Nombre tabla: %s cantidad paginas: %i \n", segmento->nombre_tabla, segmento->cant_Paginas);


	}

}

pagina_t* get_marco(int nro_marco){
	char* ptr =  memoria + (nro_marco * tamanioPagina);
	pagina_t *pag = malloc(sizeof(pagina_t));
	int desplazamiento = 0;

	memcpy(&pag->timestamp, ptr + desplazamiento, UNSIGNED_LONG_LONG);
	desplazamiento += UNSIGNED_LONG_LONG;
	memcpy(&pag->key, ptr + desplazamiento, INT);
	desplazamiento += INT;
	pag->value = malloc(tamanioValue);
	memset(pag->value,'\0',tamanioValue);
	memcpy(pag->value, ptr + desplazamiento,tamanioValue );

//	printf("Timestamp: %llu\n",pag->timestamp);
//	printf("Key: %i\n",pag->key);
//	printf("Valor: %s\n", pag->value);

	return pag;


}

void imprimir_marco(int nro_marco){
	pagina_t *pag = get_marco(nro_marco);
	printf("Timestamp: %llu\n",pag->timestamp);
	printf("Key: %i\n",pag->key);
	printf("Valor: %s\n", pag->value);

	free(pag->value);
	free(pag);
}
//
//void imprimir_marco(int nro_marco){
//	char* ptr =  memoria + (nro_marco * tamanioPagina);
//	pagina_t *pag = malloc(sizeof(pagina_t));
//	int desplazamiento = 0;
//
//	memcpy(&pag->timestamp, ptr + desplazamiento, UNSIGNED_LONG);
//	desplazamiento += UNSIGNED_LONG;
//	memcpy(&pag->key, ptr + desplazamiento, INT);
//	desplazamiento += INT;
//	pag->value = malloc(tamanioValue);
//	memset(pag->value,'\0',tamanioValue);
//	memcpy(pag->value, ptr + desplazamiento,tamanioValue );
//
//	printf("Timestamp: %lu\n",pag->timestamp);
//	printf("Key: %i\n",pag->key);
//	printf("Valor: %s\n", pag->value);
//
//	free(pag->value);
//	free(pag);
//
//}

void actualizar_pagina(  char* valor, int index_pagina, int index_segmento)
{

	tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);
	tablaPagina_t* nodo_pagia = (tablaPagina_t*) list_get(segmento->tablaPaginas,index_pagina);


	int nro_marco = nodo_pagia->nro_marco;

	bitmap[nro_marco].timestamp = obtener_timestamp();//Actualizo el time de la referencia del marco para el lru.
	bitmap[nro_marco].modificado = 1;
	bitmap[nro_marco].ocupado = 1;

	char* ptr = memoria + (nro_marco * tamanioPagina);
	int desplazamiento = 0;
	unsigned long long timestamp = obtener_timestamp();
	memcpy(ptr + desplazamiento,&(timestamp),UNSIGNED_LONG_LONG);
	desplazamiento = UNSIGNED_LONG_LONG + INT;
	memset(ptr + desplazamiento,'\0',tamanioValue);
	memcpy(ptr + desplazamiento,valor,strlen(valor));

	nodo_pagia->bit_modificado = 1;

}

pagina_t* get_valor_de_key_en_memoria(  int index_pagina, int index_segmento)
{

	tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);
	tablaPagina_t* nodo_pagia = (tablaPagina_t*) list_get(segmento->tablaPaginas,index_pagina);


	int nro_marco = nodo_pagia->nro_marco;

	char* ptr =  memoria + (nro_marco * tamanioPagina);

	pagina_t *pag = malloc(sizeof(pagina_t));

	int desplazamiento = 0;

	memcpy(&pag->timestamp, ptr + desplazamiento, UNSIGNED_LONG_LONG);
	desplazamiento += UNSIGNED_LONG_LONG;
	memcpy(&pag->key, ptr + desplazamiento, INT);
	desplazamiento += INT;
	pag->value = malloc(tamanioValue);
	memset(pag->value,'\0',tamanioValue);
	memcpy(pag->value, ptr + desplazamiento,tamanioValue );

	//printf("Timestamp: %llu\n",pag->timestamp);
	//printf("Key: %i\n",pag->key);
	//printf("Valor: %s\n", pag->value);

	return pag;

}
void imprimir_tabla_paginas(char* nombre_tabla)
{
	int index_segmento = existe_segmento(nombre_tabla);
	if(index_segmento >= 0)
	{
		tablaSegmentos_t* nodo_Seg = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);
		int j=0;
		for(;j<list_size(nodo_Seg->tablaPaginas);j++)
		{
			tablaPagina_t* nodo_pag = (tablaPagina_t*) list_get(nodo_Seg->tablaPaginas,j);
			printf("indice: %i nro pagina: %i nro maco: %i bit modificado: %i\n",j,nodo_pag->nro_pagina,nodo_pag->nro_marco,nodo_pag->bit_modificado);

		}
	}else{
		printf("No existe la tabla\n");
	}
//	int nro_pagina;
//			int nro_marco;
//			int bit_modificado;
//			int bit_ocupado;
//			int nro_segmento;
//	int j=0;
//	for(;j<cantidadDePaginas;j++)
//	{
//		printf("indice: %i nro pagina: %i nro maco: %i bit modificado: %i bit ocupado: %i nro segmento: %i\n",j,tabla_paginas[j].nro_pagina,tabla_paginas[j].nro_marco,tabla_paginas[j].bit_modificado, tabla_paginas[j].bit_ocupado, tabla_paginas[j].nro_segmento);
//
//	}
}
int agregar_entrada_tabla_pagina(int index_segmento, int bit_modificado){

	tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);

	//Busco marco libre
	int index_marco_libre = devolverMarcoLibre();

//	if(index_marco_libre == atoi(MEMORIA_FULL)){
//		return atoi(MEMORIA_FULL);
//	}

	bitmap[index_marco_libre].ocupado = 1;
	bitmap[index_marco_libre].modificado = bit_modificado;
	bitmap[index_marco_libre].timestamp = obtener_timestamp();

	tablaPagina_t* nodo_pagina = (tablaPagina_t*) malloc(sizeof(tablaPagina_t));
	if(nodo_pagina == NULL)
	{
		printf("No hay memoria suficiente para crear un nodo_pagina\n");
		return ERROR;
	}

	nodo_pagina->nro_pagina = segmento->cant_Paginas + 1;
	nodo_pagina->bit_modificado = bit_modificado;
	nodo_pagina->nro_marco = index_marco_libre;

	list_add(segmento->tablaPaginas,nodo_pagina);


    segmento->cant_Paginas += 1;

    int nro_pag = nodo_pagina->nro_pagina;

    return nro_pag;

}

void agregar_pagina(char* valores, int key, int nro_pag, int index_segmento,int bit_modificado)
{
	tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index_segmento);
	bool esLaPagina(tablaPagina_t* alguien){ return alguien->nro_pagina == nro_pag; }
	printf("nombre_tabla: %s\n", segmento->nombre_tabla);
	tablaPagina_t* nodo_pagia  = (tablaPagina_t*) list_find(segmento->tablaPaginas, (void*) esLaPagina);
	if(nodo_pagia == NULL){
		printf("No se encontro el nro de la página buscada\n");

	}else{
		pagina_t* pagina = (pagina_t*) malloc(sizeof(pagina_t));
		int nro_marco = nodo_pagia->nro_marco;

		char* timestamp;
		char* valor;


		if(bit_modificado == 0){//Si es cero entonces viene de un select
			char** campos = string_n_split(valores,2," ");
			valor = campos[0];
			timestamp = campos[1];
		}else{//Si es uno entonces viene de un insert
			valor = valores;
		}

		pagina->key = key;

		if(bit_modificado == 0){//Si es cero entonces viene de un select
			pagina->timestamp = timestamp;
		}else{//Si es uno entonces viene de un insert
			pagina->timestamp = obtener_timestamp();
		}

		pagina->value = malloc(tamanioValue);
		memset(pagina->value, '\0',tamanioValue);
		memcpy(pagina->value,valor,strlen(valor));


		//Escribo en Memoria
		char* p = memoria + (nro_marco * tamanioPagina);
		int desplazamiento1 = 0;
		memcpy(p + desplazamiento1,&(pagina->timestamp),UNSIGNED_LONG_LONG);
		desplazamiento1 += UNSIGNED_LONG_LONG;
		memcpy(p + desplazamiento1,&pagina->key,INT);
		desplazamiento1 += INT;
		//memset(p + desplazamiento1,'\0',tamanioValue);
		memcpy(p + desplazamiento1,pagina->value,tamanioValue);

		free(pagina->value);
		free(pagina);

	}

}


void atender_memoria_full_insert(int socket_kernel,char* query){

	char** campos = string_n_split(query, 4, " ");

	int bytes_enviados = aplicar_protocolo_enviar(socket_kernel,INSERT,MEMORIA_FULL);
	//printf("bytes_enviados: %i\n",bytes_enviados);

	int head;
	aplicar_protocolo_recibir(socket_kernel,&head);
	if(head == JOURNAL){
		ejecutar_journal();
	}

	int resultado = insertar_valor(campos[1], atoi(campos[2]),campos[3]);

	aplicar_protocolo_enviar(socket_kernel,JOURNAL,SUCCESSFUL);



}

void  ejecutar_comando_insert_kernel(int socket_kernel,char* query){
	char** campos = string_n_split(query, 4, " ");

//	printf("campos[0]: %s\n",campos[0]);
//	printf("campos[1]: %s\n",campos[1]);
//	printf("campos[2]: %s\n",campos[2]);
//	printf("campos[3]: %s\n",campos[3]);

	int resultado = insertar_valor(campos[1], atoi(campos[2]),campos[3]);

	if(resultado == OK){
		int bytes_enviados = aplicar_protocolo_enviar(socket_kernel,INSERT,SUCCESSFUL);
		//printf("bytes_enviados: %i\n",bytes_enviados);
	}

//	if(resultado == atoi(MEMORIA_FULL)){
//		atender_memoria_full_insert( socket_kernel, query);
//	}


}

int insertar_valor(char* nombre_tabla, int key, char* valor){
	//Buscar si la tabla existe
		//Si la tabla existe, buscar si existe la key
		//Si existe la key entonces actualizar el valor con el nuevo timestamp
		//Si la key no existe entonces agregar un nodo nuevo(validar si hay una pagina libre)

		//Si la tabla no existe agregar un nodo en la tabla de segmentos y un nodo en la tabla de paginas

		char* valor_sin_comillas = quitar_comillas(valor);

		int index_segmento;
		index_segmento = existe_segmento(nombre_tabla);
		if(index_segmento >= 0){//EL segmento SI existe

			int  index_pagina = existe_key(key,index_segmento);
			if( index_pagina >= 0){//Existe la key

				actualizar_pagina(valor_sin_comillas,index_pagina,index_segmento);
			}else{

				int nro_pag = agregar_entrada_tabla_pagina(index_segmento, 1 );
				if(nro_pag >= 0){
					agregar_pagina( valor_sin_comillas,  key,  nro_pag, index_segmento,1);
				}
//				if(nro_pag != atoi(MEMORIA_FULL)){
//					agregar_pagina( valor_sin_comillas,  key,  nro_pag, index_segmento,1);
//				}else{
//					return atoi(MEMORIA_FULL);
//				}

			}

		}else{//EL segmento no existe
			int rta = agregar_segmento_pagina(nombre_tabla,  key,  valor_sin_comillas,1);
//			if(rta == atoi(MEMORIA_FULL)){
//				return atoi(MEMORIA_FULL);
//			}
		}

		return OK;
}

char* quitar_comillas(char* valor){
	char* valor_sin_comillas;
	int lengt_valor = strlen(valor);
	//printf("lengt_valor: %i\n",lengt_valor);
	valor_sin_comillas = string_substring(valor,1,(lengt_valor-2));
	//printf("tamaño valor_sin_comillas: %i\n",strlen(valor_sin_comillas));
	//printf("valor_sin_comillas: %s\n",valor_sin_comillas);
	return valor_sin_comillas;
}
int  ejecutar_comando_insert(char* nombre_tabla, int key, char* valor)
{

	int result = insertar_valor(nombre_tabla,key,valor);
	if(result == OK){
		printf("Comando ejecutado correctamente\n");
	}

//	if(result == atoi(MEMORIA_FULL)){
//		printf("La memoria se encuentra FULL. Debe realizar un Journal\n");
//	}


}

int existe_key(int key, int index)
{
	int existe = ERROR;
	tablaSegmentos_t* segmento = (tablaSegmentos_t*) list_get(tabla_segmentos,index);
	int j=0;
	for(;j<list_size(segmento->tablaPaginas);j++)
	{
		tablaPagina_t* nodo_pagia = (tablaPagina_t*) list_get(segmento->tablaPaginas,j);

		char* ptr = memoria + (nodo_pagia->nro_marco * tamanioPagina);

		pagina_t *pagina = malloc(sizeof(pagina_t));
		int desplazamiento = 0;

		desplazamiento += UNSIGNED_LONG_LONG;
		memcpy(&pagina->key, ptr + desplazamiento, INT);

		if(pagina->key == key)
		{
			existe = j;
			free(pagina);
			return existe;
		}

		free(pagina);


	}

	return existe;


}

int ejecutar_comando_create(char* query){


	int head;
	char * resultado = NULL;



	pthread_mutex_lock(&mutex_socket_lfs);

	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,CREATE,query);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);

	pthread_mutex_unlock(&mutex_socket_lfs);



	int result = atoi(resultado);

	switch(result)
	 {
		 case  1: printf("La tabla  fue creada exitosamente.\n");
				  break;
		 case -1: printf("La creación de la tabla falló al momento de crear el directorio de la tabla.\n");
				  break;
		 case -2: printf("La creación de la tabla falló al momento de crear el archivo metadata de la tabla.\n");
				  break;
		 case -3: printf("La creación de la tabla falló al momento de crear las particiones de la tabla.\n");
				  break;
		 case -4: printf("La creación de la tabla falló al momento de agregar la tabla en la memtable.\n");
				  break;
		 case -5: printf("La tabla ya existe.\n");
				  break;
		 default: break;
	 }

	//add_nuevo_script(query);

	free(query); query=NULL;
	free(resultado); resultado=NULL;

	return OK;

}

//void ejecutar_comando_create(char* nombre_tabla, char* consistency, char* cant_part, char* compact_time){
//
//
//	int head;
//	char * resultado = NULL;
//	char* query = malloc(TAM_MAX_QUERY);
//	memset(query,'\0',TAM_MAX_QUERY);
//	strcpy(query,"create");
//	if(nombre_tabla != NULL){
//		string_append(&query ,"|");
//		string_append(&query ,nombre_tabla);
//	}
//
//	string_append(&query ," ");
//	string_append(&query ,consistency);
//
//	string_append(&query ,"|");
//	string_append(&query ,cant_part);
//
//	string_append(&query ,"|");
//	string_append(&query ,compact_time);
//
//
//	pthread_mutex_lock(&mutex_socket_lfs);
//
//	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,CREATE,query);
//	resultado = aplicar_protocolo_recibir(fdLfs,&head);
//
//	pthread_mutex_unlock(&mutex_socket_lfs);
//
//
//
//	int result = atoi(resultado);
//
//	switch(result)
//	 {
//		 case  1: printf("La tabla %s fue creada exitosamente.\n", nombre_tabla);
//				  break;
//		 case -1: printf("La creación de la tabla falló al momento de crear el directorio de la tabla.\n");
//				  break;
//		 case -2: printf("La creación de la tabla falló al momento de crear el archivo metadata de la tabla.\n");
//				  break;
//		 case -3: printf("La creación de la tabla falló al momento de crear las particiones de la tabla.\n");
//				  break;
//		 case -4: printf("La creación de la tabla falló al momento de agregar la tabla en la memtable.\n");
//				  break;
//		 case -5: printf("La tabla %s ya existe.\n", nombre_tabla);
//				  break;
//		 default: break;
//	 }
//
//	//add_nuevo_script(query);
//
//	free(query); query=NULL;
//	free(resultado); resultado=NULL;
//
//}

void ejecutar_comando_create_kernel(int socket_kernel, char* consulta){
	//printf("consulta: %s\n",consulta);
	char * resultado = NULL;
	int head;
	pthread_mutex_lock(&mutex_socket_lfs);
	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,CREATE,consulta);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);
	pthread_mutex_unlock(&mutex_socket_lfs);
	//printf("head: %i\n",head);
	//printf("resultado: %s\n",resultado);
	int	bytes_enviados_k = aplicar_protocolo_enviar(socket_kernel,CREATE,resultado);
	//printf("bytes_enviadosa ker: %i\n",bytes_enviados_k);
}

//void ejecutar_comando_describe_kernel(int socket_kernel, char* consulta){
//
//	int head;
//	char * resultado = NULL;
//	printf("query es: %s\n",consulta);
//	char** parametros = string_n_split(consulta, 2, " ");
//	char* comando = parametros[0];
//	char* nombre_tabla = parametros[1];
//	printf("parametros[0] es: %s\n",parametros[0]);
//	printf("parametros[1] es: %s\n",parametros[1]);
//
//	char* query = malloc(TAM_MAX_QUERY);
//	memset(query,'\0',TAM_MAX_QUERY);
//	strcpy(query,comando);
//	if(nombre_tabla != NULL){
//		string_append(&query ,"|");
//		string_append(&query ,nombre_tabla);
//	}
//
//	pthread_mutex_lock(&mutex_socket_lfs);
//
//	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DESCRIBE,query);
//	resultado = aplicar_protocolo_recibir(fdLfs,&head);
//
//	pthread_mutex_unlock(&mutex_socket_lfs);
//
//	int	bytes_enviados_to_kernel = aplicar_protocolo_enviar(socket_kernel,DESCRIBE,resultado);
//	printf("bytes_enviados_to_kernel: %i\n", bytes_enviados_to_kernel);
//	printf("resultado: %s\n", resultado);
//
//}




void ejecutar_comando_describe_kernel(int socket_kernel, char* consulta){

	int head;
	char * resultado = NULL;
	//printf("query es: %s\n",consulta);
//	char** parametros = string_n_split(consulta, 2, " ");
//	char* comando = parametros[0];
//	char* nombre_tabla = parametros[1];
//	printf("parametros[0] es: %s\n",parametros[0]);
//
//
//	char* query = malloc(TAM_MAX_QUERY);
//	memset(query,'\0',TAM_MAX_QUERY);
//	strcpy(query,comando);
//	if(nombre_tabla != NULL){
//		string_append(&query ,"|");
//		string_append(&query ,nombre_tabla);
//	}

	pthread_mutex_lock(&mutex_socket_lfs);

	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DESCRIBE,consulta);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);

	pthread_mutex_unlock(&mutex_socket_lfs);

	int	bytes_enviados_to_kernel = aplicar_protocolo_enviar(socket_kernel,DESCRIBE,resultado);
	//printf("bytes_enviados_to_kernel: %i\n", bytes_enviados_to_kernel);
	//printf("resultado: %s\n", resultado);

}

void drop_tabla_memoria(char* nombre_tabla){


	int index_segmento = existe_segmento(nombre_tabla);
	//printf("index_segmento: %i\n", index_segmento);
	if(index_segmento >= 0)
	{
		tablaSegmentos_t* tabla = (tablaSegmentos_t*) list_remove(tabla_segmentos,index_segmento);
		int i;
		for(i=0;i<list_size(tabla->tablaPaginas);i++){
			tablaPagina_t* reg = list_remove(tabla->tablaPaginas,i);
			bitmap[reg->nro_marco].modificado = 0;
			bitmap[reg->nro_marco].ocupado = 0;
			bitmap[reg->nro_marco].timestamp = 0;

			free(reg);

		}

		free(tabla);
	}
}
void ejecutar_comando_drop_kernel(int socket_kernel, char* consulta){

	char** campos = string_n_split(consulta,2," ");
	drop_tabla_memoria(campos[1]);

	int head;
	char * resultado = NULL;
	//printf("query es: %s\n",consulta);

	pthread_mutex_lock(&mutex_socket_lfs);

	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DROP,consulta);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);

	pthread_mutex_unlock(&mutex_socket_lfs);

	int	bytes_enviados_to_kernel = aplicar_protocolo_enviar(socket_kernel,DROP,resultado);
	//printf("bytes_enviados_to_kernel: %i\n", bytes_enviados_to_kernel);
	//printf("resultado: %s\n", resultado);


}

int ejecutar_comando_drop(char* query){

	int head;
	char * resultado = NULL;


	char** campos = string_n_split(query,2," ");
	drop_tabla_memoria(campos[1]);

	pthread_mutex_lock(&mutex_socket_lfs);

	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DROP,query);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);

	pthread_mutex_unlock(&mutex_socket_lfs);

	printf("%s\n",resultado);

	free(query); query=NULL;
	free(resultado); resultado=NULL;

	return OK;


}

void ejecutar_comando_describe(char* query)
{//Consulta al LFS

	int head;
	char * resultado = NULL;


	pthread_mutex_lock(&mutex_socket_lfs);

	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DESCRIBE,query);
	resultado = aplicar_protocolo_recibir(fdLfs,&head);

	pthread_mutex_unlock(&mutex_socket_lfs);

	printf("%s\n",resultado);

	free(query); query=NULL;
	free(resultado); resultado=NULL;


}
//void ejecutar_comando_describe(char* nombre_tabla)
//{//Consulta al LFS
//
//	int head;
//	char * resultado = NULL;
//	char* query = malloc(TAM_MAX_QUERY);
//	memset(query,'\0',TAM_MAX_QUERY);
//	strcpy(query,"describe");
//	if(nombre_tabla != NULL){
//		string_append(&query ,"|");
//		string_append(&query ,nombre_tabla);
//	}
//
//
//
//	pthread_mutex_lock(&mutex_socket_lfs);
//
//	int	bytes_enviados = aplicar_protocolo_enviar(fdLfs,DESCRIBE,query);
//	resultado = aplicar_protocolo_recibir(fdLfs,&head);
//
//	pthread_mutex_unlock(&mutex_socket_lfs);
//
//	printf("%s\n",resultado);
//
//	//add_nuevo_script(query);
//
//	free(query); query=NULL;
//	free(resultado); resultado=NULL;
//
//
//}
int ejecutar_comando(char* comando) {
	int ret = 0;
	char* comando_listo = comando_preparado(comando);
	string_to_upper(comando_listo);
	char** parametros = string_n_split(comando_listo, 6, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_AYUDA)) {
		//printf("Se ejecuto comando ayuda\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_SELECT)) {

		int key = NULL;
		if(parametros[2] != NULL){
			key = strtol(parametros[2],NULL,10);
		}
		ret = ejecutar_comando_select(parametros[1] , key, comando_listo);
		return ret;

	} else if (string_equals_ignore_case(parametros[0], COMANDO_INSERT)) {

		char** parametrosInsert = string_n_split(comando_listo, 4, " ");
		if (parametrosInsert[1] == NULL || parametrosInsert[2] == NULL || parametrosInsert[3] == NULL) {
			printf("Faltan ingresar parametros");
			return ERROR;
		}
		ret = ejecutar_comando_insert(parametrosInsert[1], strtol(parametrosInsert[2], NULL, 10), parametrosInsert[3]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_CREATE)) {
		//ejecutar_comando_create(parametros[1],parametros[2],parametros[3],parametros[4]);
		ret = ejecutar_comando_create(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DESCRIBE)) {
		//printf("el nombre de la tabla es %s\n",parametros[1]);
//		if(parametros[1]!= NULL )
//		{
//			ejecutar_comando_describe(parametros[1]);
//		}else
//		{
//			ejecutar_comando_describe("");
//		}
		ejecutar_comando_describe(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DROP)) {
		ret = ejecutar_comando_drop(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_JOURNAL)) {
		ejecutar_comando_journal();
		return ret;
	}  else if (string_equals_ignore_case(parametros[0], COMANDO_PAGINAS)) {
		if (parametros[1] == NULL ) {
					printf("Faltan ingresar parametros");
					return ERROR;
		}
		imprimir_tabla_paginas(parametros[1]);
		return EXIT_SUCCESS;

	} else if (string_equals_ignore_case(parametros[0], COMANDO_BITMAP)) {
		imprimir_bit_map();
		return EXIT_SUCCESS;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_SEGMENTOS)) {
		imprimir_tabla_segmentos();
		return EXIT_SUCCESS;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_MARCO)) {
		if (parametros[1] == NULL ) {
							printf("Faltan ingresar parametros");
							return ERROR;
				}
		imprimir_marco(atoi(parametros[1]));
		return EXIT_SUCCESS;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_GOSSIPING)) {
		exec_gossiping();
		return EXIT_SUCCESS;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_T_GOSSIPING)) {
		mostrar_tabla_gossiping();
		return EXIT_SUCCESS;

	}
	else {
		log_error(logger,"Comando incorrecto ingrese ayuda para una lista de comandos validos o salir para cerrar el sistema");
		return ERROR;
	}
	return 0;
}

void* reservarMemoria(int size) {

	void *puntero = malloc(size);
	if(puntero == NULL) {
		fprintf(stderr, "Error al reservar %d bytes de memoria\n", size);
		exit(ERROR);
	}
	return puntero;
}

int existe_segmento(char* nombre_tabla) {

	int i;
	tablaSegmentos_t *aux = NULL;
	if(tabla_segmentos != NULL){
		for (i = 0; i < list_size(tabla_segmentos); i++){
			aux = (tablaSegmentos_t*) list_get(tabla_segmentos, i);


			if(!strcmp(aux->nombre_tabla, nombre_tabla)){
				return i;
			}

		}
	}

	return ERROR; // no se encontró el segmento
}

//int get_nro_ultimo_segmento() {
//
//	int nro_seg_max = 0;
//
//	int i = 0;
//	tablaSegmentos_t *aux = NULL;
//	for (; i < list_size(tabla_segmentos); i++){
//		aux = (tablaSegmentos_t*) list_get(tabla_segmentos, i);
//
//		if(aux->nro_segmento > nro_seg_max){
//			nro_seg_max = aux->nro_segmento ;
//		}
//
//	}
//
//
//	return nro_seg_max;
//}


void imprimir_bit_map()
{
	int i = 0;

	for(; i < cantidad_de_marcos; i++){
		printf("el marco %i esta %i \n", i,bitmap[i].ocupado);

	}

}

int retornar_marco_por_lru(){
//Retorno MEMORIA_FULL si estan todos ocupados y modificados
	//Falta desarrollar
	int marco = -1;
	int i = 0;
	unsigned long long time_menor = obtener_timestamp();
	for(; i < cantidad_de_marcos; i++){
		if(bitmap[i].modificado == 0){
			if(bitmap[i].timestamp < time_menor){
				time_menor = bitmap[i].timestamp;
				marco = i;

			}

		}

	}
	return marco;
}

int devolverMarcoLibre()
{
	//Si hay marcos libre retorno el primero
	int marco;
	int i = 0;
	for(; i < cantidad_de_marcos; i++){
		if(bitmap[i].ocupado == 0){
			marco = i;
			return marco;
		}

	}


	//Si todos estan ocupados entonces tengo que buscar alguno para reemplazar.
	//Aplico lru sobre todos los que no estan modificados.
	marco = retornar_marco_por_lru();


	//Si estan todos ocupados y modificados entonces esta a FULL
	//marco es MEMORIA_FULL si esta todo ocupado y modificado

	if(marco == -1){//Memoria FULL

		ejecutar_comando_journal();

		int j=0;
		for(; j < cantidad_de_marcos; j++){
			if(bitmap[j].ocupado == 0){
				marco = j;
				return marco;
			}

		}

	}

	return marco;
}


int agregar_segmento_pagina(char* nombre_tabla, int key, char* valores,int bit_modificado)
{

	char* timestamp;
	char* valor;
	if(bit_modificado == 0){//Si es cero entonces viene de un select
		char** campos = string_n_split(valores,2," ");
		valor = campos[0];
		timestamp = campos[1];
		printf("timestamp: %s\n",timestamp);
	}else{//Si es uno entonces viene de un insert
		valor = valores;
	}


	pagina_t* pagina = (pagina_t*) malloc(tamanioPagina);

	pagina->key = key;

	if(bit_modificado == 0){//Si es cero entonces viene de un select
		pagina->timestamp = strtoull(timestamp,NULL,10);
	}else{
		pagina->timestamp = obtener_timestamp();
	}


	pagina->value = malloc(tamanioValue);
	memset(pagina->value,'\0',tamanioValue);
	memcpy(pagina->value,valor,strlen(valor));

	//Busco marco libre
	int index_marco_libre = devolverMarcoLibre();
//	if(index_marco_libre == atoi(MEMORIA_FULL)){
//		return atoi(MEMORIA_FULL);
//	}
	bitmap[index_marco_libre].ocupado = 1;
	bitmap[index_marco_libre].modificado = bit_modificado;
	bitmap[index_marco_libre].timestamp = obtener_timestamp();

	//Me posiciono para escribir en la memoria
	char* posicion_mp = memoria + (index_marco_libre * tamanioPagina);

	int desplazamiento = 0;

	//Escribo en la Memoria
	memcpy(posicion_mp + desplazamiento,&(pagina->timestamp),UNSIGNED_LONG_LONG);
	desplazamiento += UNSIGNED_LONG_LONG;
	memcpy(posicion_mp + desplazamiento,&pagina->key,INT);
	desplazamiento += INT;
	//memset(posicion_mp + desplazamiento,'\0',tamanioValue);
	memcpy(posicion_mp + desplazamiento,pagina->value,tamanioValue );


	////////Creo el nodo para la tabla de pagina////////////////////
	//Creo el nodo para la tabla de pagina
	tablaPagina_t* nodo_pagina = (tablaPagina_t*) malloc(sizeof(tablaPagina_t));
	if(nodo_pagina == NULL)
	{
		printf("No hay memoria para un registro en la tabla de paginas\n");
	}
	nodo_pagina->nro_pagina = 1;
	nodo_pagina->nro_marco = index_marco_libre;
	nodo_pagina->bit_modificado = bit_modificado;

	/// Creación del nodo segmento
	tablaSegmentos_t* nodo_segmento = (tablaSegmentos_t*) malloc(sizeof(tablaSegmentos_t));
	if(nodo_segmento == NULL)
	{
		printf("No hay memoria para un registro en la tabla de segmentos\n");
	}

	strcpy(nodo_segmento->nombre_tabla,nombre_tabla);
	nodo_segmento->cant_Paginas = 1;
	nodo_segmento->tablaPaginas = list_create();
	list_add(nodo_segmento->tablaPaginas,nodo_pagina);

	list_add(tabla_segmentos,nodo_segmento);


	free(pagina->value);
	free(pagina);

	return OK;
}

unsigned long long obtener_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = ((unsigned long long)tv.tv_sec*1e3) + ((unsigned long long)tv.tv_usec/1000);
	return timestamp;
}


/************************PROTOCOLO DE COMUNICACION**************************************/


// SEREALIZAR: Del mensaje listo para enviar, al buffer
void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer = NULL;

	switch(head) {
	// CASE 1: El mensaje es un texto (char*)
	case DESCRIBE: case CREATE: case TABLA_GOSSIPING: case INSERT: case SELECT: case JOURNAL: case DROP:{
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
		break;
	}

  } // fin switch head
	return buffer;
}
// DESEREAILZAR: Del buffer, al mensaje listo para recibir
void * deserealizar(int head, void * buffer, int tamanio){

	void * mensaje = NULL;

	switch(head){
	// CASE 1: El mensaje es un texto (char*)
	case TAMANIO_VALUE: case DESCRIBE: case CREATE: case INSERT: case SELECT: case JOURNAL: case DROP: case TABLA_GOSSIPING:{
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
			case DESCRIBE: case CREATE: case TABLA_GOSSIPING: case INSERT: case SELECT: case JOURNAL: case DROP:{
				if(mensaje != NULL)
					tamanio = strlen((char*) mensaje) + 1;
				else
					tamanio = 0;
				//printf("El tamanio de query es %i\n",tamanio);
				break;
			}

	} // fin switch head

	return tamanio;
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






/************************************************************************/
////Retorna index de la entrada de la tabla de páginas
//int buscar_pagina_libre()
//{
//	int index_pagina_libre = -1;
//	int j=0;
//	for(;j<cantidadDePaginas;j++)
//	{
//		if(tabla_paginas[j].bit_ocupado == 0)
//		{
//			index_pagina_libre = j;
//			return index_pagina_libre;
//		}
//	}
//
//	return index_pagina_libre;//Si retorna -1 es porque no hay lugar, tendre que aplicar el alg de reemplazo
//}




