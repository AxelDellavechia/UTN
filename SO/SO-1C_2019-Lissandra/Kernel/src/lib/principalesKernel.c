
#include "principalesKernel.h"


void crearHilos() {

	uint16_t i,iret;
	uint16_t nivelMultiprocesamiento = configFile->multiprocesamiento;
	pthread_t hilos_Procesadores[nivelMultiprocesamiento];
	//sem_init (&mutex_conexion,0,1);
	for (i=0;i<nivelMultiprocesamiento;i++){

		iret = pthread_create(&hilos_Procesadores[i],NULL,(void *)consumidor,NULL);
		if(iret!=0){
			log_error(logger,"Error al crear un hilo consumidor\n");
			printf("Error al crear un hilo consumidor\n");

		}

	}



	pthread_t hilo_consola, hilo_gossiping, hilo_actualizar_info_tablas, hilo_metricas, hilo_inotify;


	pthread_create(&hilo_gossiping, NULL, (void*) gossiping, NULL);
	pthread_create(&hilo_actualizar_info_tablas, NULL, (void*) actualizar_info_tablas, NULL);
	pthread_create(&hilo_metricas, NULL, (void*) logear_metricas, NULL);
	pthread_create(&hilo_inotify,NULL,(void*)actualizar_file_config,NULL);
	pthread_create(&hilo_consola, NULL, (void*) consola, NULL);

	pthread_join(hilo_metricas, NULL);
	pthread_join(hilo_gossiping, NULL);
	pthread_join(hilo_actualizar_info_tablas, NULL);
	pthread_join(hilo_inotify, NULL);



	for (i=0;i<nivelMultiprocesamiento;i++){
		pthread_join(hilos_Procesadores[i],NULL);
	}

	pthread_join(hilo_consola,NULL);
}


void iniciarEstructuras(){

	lista_scripts = list_create();
	lista_memorias = list_create();
	lista_sc = list_create();
	lista_shc = list_create();
	cola_ec = queue_create();
	lista_tablas = list_create();
	cola_listos = queue_create();


	metricas = malloc(sizeof(metricas_t));
	metricas->cant_reads = 0;
	metricas->cant_writes = 0;
	metricas->tiempo_reads = 0;
	metricas->tiempo_writes = 0;
	metricas->cant_inserts_select_total = 0;
	metricas->memorias_metrics = list_create();

	sem_init (&contador_cola_listos,0,0);
	pthread_mutex_init(&mutex_cola_listos,NULL);
	pthread_mutex_init(&mutex_lista_tablas,NULL);
	pthread_mutex_init(&mutex_metricas,NULL);
	pthread_mutex_init(&mutex_file_config,NULL);

	//Agrego la memoria que conozca a la lista de gossiping

//	memoria_t* memoria1 = (memoria_t*) malloc(sizeof(memoria_t));
//
//	int tam_ip = string_length(configFile->ipMemoria);
//
//	memoria1->ip = (char*)malloc(tam_ip + 1);
//	memset(memoria1->ip,tam_ip +1,'\0');
//
//	memcpy(memoria1->ip,configFile->ipMemoria,tam_ip);
//	memoria1->ip[tam_ip] = '\0';
//	memoria1->puerto = atoi(configFile->puertoMemoria);
//	list_add(lista_memorias,memoria1);
}

void vaciar_info_tablas(){

	pthread_mutex_lock(&mutex_lista_tablas);
	list_clean(lista_tablas);
	pthread_mutex_unlock(&mutex_lista_tablas);

}

int calcular_cant_tablas(char** tablas){
	int j;
	for(j=0;j<100;j++){
		if(tablas[j] == NULL){
			//printf("tamanio: %i\n",j);
			return j;
		}
	}
	return 100;
}



void actualizar_file_config()
{

	char buffer[BUF_LEN];
	char* directorio = RUTA_CONFIG_KERNEL;
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

//				int retardo_anterior=configFile->retardoCicloEjecucion;
//				int metadata_refresh_anterior=configFile->refreshMetadata;
//				int quantum_anterior = configFile->quantum;

				load_config_file(RUTA_CONFIG_KERNEL);



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

void mostrar_metricas_consola(){


	pthread_mutex_lock(&mutex_metricas);
	unsigned long long read_latency = metricas->tiempo_reads / metricas->cant_reads;
	unsigned long long write_latency = metricas->tiempo_writes / metricas->cant_writes;

	printf("*********************METRICAS****************************\n");
	printf("read_latency = %llu\n",read_latency);
	printf("write_latency = %llu\n",write_latency);
	int i;
	for(i=0;i<list_size(metricas->memorias_metrics);i++){

		memorias_metrics_t* mem = list_get(metricas->memorias_metrics,i);
		unsigned long long promedio = mem->cant_inserts_y_selects / metricas->cant_inserts_select_total;
		printf("Nro Memoria= %i Memory load = %llu\n",mem->nro_memoria,promedio);
	}

	//Tengo que logear tambien cuando me piden las metricas por consola.
	mostrar_metricas_log();

	pthread_mutex_unlock(&mutex_metricas);
}

void mostrar_metricas_log(){



	unsigned long long read_latency = metricas->tiempo_reads / metricas->cant_reads;
	unsigned long long write_latency = metricas->tiempo_writes / metricas->cant_writes;

	log_info(logger,"*********************METRICAS****************************\n");
	log_info(logger,"read_latency = %llu\n",read_latency);
	log_info(logger,"write_latency = %llu\n",write_latency);
	int i;
	for(i=0;i<list_size(metricas->memorias_metrics);i++){

		memorias_metrics_t* mem = list_get(metricas->memorias_metrics,i);
		unsigned long long promedio = mem->cant_inserts_y_selects / metricas->cant_inserts_select_total;
		log_info(logger,"Nro Memoria= %i Memory load = %llu\n",mem->nro_memoria,promedio);
	}


}

void reset_metricas(){


	metricas->tiempo_reads = 0;
	metricas->tiempo_writes = 0;
	metricas->cant_reads = 0;
	metricas->cant_writes = 0;
	metricas->cant_inserts_select_total = 0;
	list_clean(metricas->memorias_metrics);
}
void logear_metricas(){


//	while(TRUE)
//	{
//		usleep(30000000);
//
//		pthread_mutex_lock(&mutex_metricas);
//		mostrar_metricas_log();
//		reset_metricas();
//		pthread_mutex_unlock(&mutex_metricas);
//	}



}
void actualizar_info_tablas(){

	vaciar_info_tablas();

	int fdMemoria = nuevoSocket();
	if (conectarConMemoria(fdMemoria)){ // Conexión con Memoria
		handshake_cliente(fdMemoria, "K");
		char* msg = malloc(10);
		strcpy(msg, "DESCRIBE");
		int bytes = aplicar_protocolo_enviar(fdMemoria,DESCRIBE,msg);
		int* head = malloc(INT);
		char* buffer = aplicar_protocolo_recibir(fdMemoria,head);

		cerrarSocket(fdMemoria);

		free(head);

		char** tablas  = string_split(buffer,  "\n");
		int cant = calcular_cant_tablas(tablas);


		int j;
		for(j=0;j<cant ;j++){


				//printf("tablas[j]: %s\n",tablas[j]);
				char** campos  = string_n_split(tablas[j], 4, " ");
				t_config_MetaData_tabla* meta_tabla = malloc(sizeof(t_config_MetaData_tabla));
				int tam_nombre = strlen(campos[0]);

				meta_tabla->nombre_tabla = malloc(tam_nombre + 1);
				strcpy(meta_tabla->nombre_tabla,campos[0]);


				meta_tabla->consistency = malloc(4);
				strcpy(meta_tabla->consistency,campos[1]);


				meta_tabla->partitions = atoi(campos[2]);
				meta_tabla->compaction_time = atoi(campos[3]);

				pthread_mutex_lock(&mutex_lista_tablas);
				list_add(lista_tablas,meta_tabla);
				pthread_mutex_unlock(&mutex_lista_tablas);


		}

		free(buffer); buffer=NULL;

//		int z;
//		for(z=0;z<cant;z++){
//			t_config_MetaData_tabla* tab = list_get(lista_tablas,z);
//
//			printf("nombre tabla %s\n", tab->nombre_tabla);
//			printf("consistencia %s\n", tab->consistency);
//			printf("compactacion %i\n", tab->compaction_time);
//			printf("particiones %i\n\n\n", tab->partitions);
//		}

	}



}

//void actualizar_info_tablas(){
//
//	vaciar_info_tablas();
//
//	int fdMemoria = nuevoSocket();
//	if (conectarConMemoria(fdMemoria)){ // Conexión con Memoria
//		handshake_cliente(fdMemoria, "K");
//		char* msg = malloc(10);
//		strcpy(msg, "DESCRIBE");
//		int bytes = aplicar_protocolo_enviar(fdMemoria,DESCRIBE,msg);
//		int* head = malloc(INT);
//		char* buffer = aplicar_protocolo_recibir(fdMemoria,head);
//
//		cerrarSocket(fdMemoria);
//
//		free(head);
//
//		char** mensaje  = string_n_split(buffer, 2, "*");
//		int cabeza = atoi(mensaje[0]);
//		char* cuerpo = mensaje[1];
//		char** tablas  = string_n_split(cuerpo, cabeza, "\n");
//		int j;
//		for(j=0;j<cabeza;j++){
//
//			char** campos  = string_n_split(tablas[j], 4, "|");
//			t_config_MetaData_tabla* meta_tabla = malloc(sizeof(t_config_MetaData_tabla));
//			int tam_nombre = strlen(campos[0]);
//
//			meta_tabla->nombre_tabla = malloc(tam_nombre + 1);
//			strcpy(meta_tabla->nombre_tabla,campos[0]);
//
//
//			meta_tabla->consistency = malloc(4);
//			strcpy(meta_tabla->consistency,campos[1]);
//
//
//			meta_tabla->partitions = atoi(campos[2]);
//			meta_tabla->compaction_time = atoi(campos[3]);
//
//			list_add(lista_tablas,meta_tabla);
//		}
//
//		free(buffer); buffer=NULL;
//
//		int z;
//		for(z=0;z<cabeza;z++){
//			t_config_MetaData_tabla* tab = list_get(lista_tablas,z);
//
////			printf("nombre tabla %s\n", tab->nombre_tabla);
////			printf("consistencia %s\n", tab->consistency);
////			printf("compactacion %i\n", tab->compaction_time);
////			printf("particiones %i\n\n\n", tab->partitions);
//		}
//
//	}
//
//	printf("Fin Ejecuto describe global\n");
//
//}

//
//void gossiping()
//{
//	//Formato en que se recibe la tabla gossiping con dos memorias (NroMemoria-IP-Puerto)
//	//2*1-127.0.0.1-8000/2-127.0.0.1-8002/
//	//printf("hilo gossiping\n");
//	int fdMemoria = nuevoSocket();
//	if (conectarConMemoria(fdMemoria)){ // Conexión con Memoria
//
//		handshake_cliente(fdMemoria, "K");
//
//		int bytes = aplicar_protocolo_enviar(fdMemoria,TABLA_GOSSIPING,NULL);
//		int* head = malloc(INT);
//		char* buffer = aplicar_protocolo_recibir(fdMemoria,head);
//
//		//char* buffer = malloc(37);
//	    //strcpy(buffer,"2*1-127.0.0.1-8000/2-127.0.0.1-8002/");
//
//		int tam_buffer = strlen(buffer);
//
//		buffer[tam_buffer -1] = '\0';
//
//		char** mensaje  = string_n_split(buffer, 2, "*");
//		int cabeza = atoi(mensaje[0]);
//		char* cuerpo = mensaje[1];
//		char** memorias  = string_n_split(cuerpo, cabeza, "/");
//		//free(head);
//		int j;
//
//		for(j=0;j<cabeza;j++){
//			char** partes = string_n_split(memorias[j], 3, "-");
//			char* nro_memoria = partes[0];
//			char* ip = partes[1];
//			char* puerto = partes[2];
//
//			int tam_ip = strlen(ip);
//
//			memoria_t* nodo = malloc(sizeof(memoria_t)) ;
//			nodo->ip = malloc(tam_ip +1);
//
//			strcpy(nodo->ip,ip);
//			nodo->puerto = atoi(puerto);
//			nodo->memory_number = atoi(nro_memoria);
//
//			list_add(lista_memorias,nodo);
//
//		}
//
//		int i;
//		for(i=0;i<list_size(lista_memorias);i++){
//			memoria_t* registro = list_get(lista_memorias,i);
//
////			printf("NRO MEMORIA: %i\n", registro->memory_number);
////			printf("IP: %s\n", registro->ip);
////			printf("PUERTO: %i\n\n", registro->puerto);
//
//		}
//
//		cerrarSocket(fdMemoria);
//
//	}
//
//}
//

int calcular_cantidad_memorias(char** memorias){

	int j;
	for(j=0;j<100;j++){
		if(memorias[j] == NULL){
			//printf("cant memorias: %i\n",j);
			log_info("cant memorias: %i\n",j);
			return j;
		}
	}
	return 100;
}


void mostrar_tabla_gossiping(){

	int y;
	for(y=0;y<list_size(lista_memorias);y++){
		memoria_t* registro = list_get(lista_memorias,y);


		printf("NRO MEMORIA: %i\n", registro->memory_number);
		printf("IP: %s\n", registro->ip);
		printf("PUERTO: %i\n\n", registro->puerto);

	}
}

void gossiping()
{
	exec_gossiping();
	while (TRUE) {
		usleep(configFile->retardoGossiping * 1000);
		exec_gossiping();
	}
}
void exec_gossiping()
{
	//Formato en que se recibe la tabla gossiping con dos memorias (NroMemoria-IP-Puerto)
	//2*1-127.0.0.1-8000/2-127.0.0.1-8002/
	//printf("hilo gossiping\n");

	t_list* lista_memorias_aux = list_create();

	//Siempre hago gossiping con la memoria seed
	int fdMemoria_seed;

	fdMemoria_seed = nuevoSocket();
	if(conectarConMemoria(fdMemoria_seed)){
		handshake_cliente(fdMemoria_seed, "K");

		int bytes = aplicar_protocolo_enviar(fdMemoria_seed,TABLA_GOSSIPING,NULL);
		int* head = malloc(INT);
		char* buffer = aplicar_protocolo_recibir(fdMemoria_seed,head);

		int tam_buffer = strlen(buffer);

		buffer[tam_buffer -1] = '\0';

		char** tablas = string_split(buffer,"\n");
		int cant_mems = calcular_cantidad_memorias(tablas);

		free(head);
		int j;

		for(j=0;j<cant_mems;j++){
			char** partes = string_n_split(tablas[j], 3, "-");
			char* nro_memoria = partes[0];
			char* ip = partes[1];
			char* puerto = partes[2];

			int tam_ip = strlen(ip);

			memoria_t* nodo = malloc(sizeof(memoria_t)) ;
			nodo->ip = malloc(tam_ip +1);

			strcpy(nodo->ip,ip);
			nodo->puerto = atoi(puerto);
			nodo->memory_number = atoi(nro_memoria);
//			int y;
//			for(y=0;y<list_size(lista_memorias);y++){
//				memoria_t* registro = list_get(lista_memorias,y);
//
//
////			printf("NRO MEMORIA: %i\n", registro->memory_number);
////			printf("IP: %s\n", registro->ip);
////			printf("PUERTO: %i\n\n", registro->puerto);
//
//			}

			list_add(lista_memorias_aux,nodo);

		}

	}



	int k;
	for(k=0;k<list_size(lista_memorias);k++){

		memoria_t* mem = list_get(lista_memorias,k);
		//si no es el seed le hago gossiping porque ya hice gossiping con el seed
		if(!(configFile->puertoMemoria == mem->puerto & strcmp(configFile->ipMemoria,mem->ip) == 0)){

			int fdMemoria = nuevoSocket();
			if (conectarConMemoria_gossiping(fdMemoria,mem->ip, mem->puerto)){ // Conexión con Memoria

				handshake_cliente(fdMemoria, "K");

				int bytes = aplicar_protocolo_enviar(fdMemoria,TABLA_GOSSIPING,NULL);
				int* head = malloc(INT);
				char* buffer = aplicar_protocolo_recibir(fdMemoria,head);

				int tam_buffer = strlen(buffer);

				buffer[tam_buffer -1] = '\0';

				char** tablas = string_split(buffer,"\n");
				int cant_mems = calcular_cantidad_memorias(tablas);

				free(head);
				int j;

				for(j=0;j<cant_mems;j++){
					char** partes = string_n_split(tablas[j], 3, "-");
					char* nro_memoria = partes[0];
					char* ip = partes[1];
					char* puerto = partes[2];

					int tam_ip = strlen(ip);

					memoria_t* nodo = malloc(sizeof(memoria_t)) ;
					nodo->ip = malloc(tam_ip +1);

					strcpy(nodo->ip,ip);
					nodo->puerto = atoi(puerto);
					nodo->memory_number = atoi(nro_memoria);

					list_add(lista_memorias_aux,nodo);

				}

//				int i;
//				for(i=0;i<list_size(lista_memorias_aux);i++){
//					memoria_t* registro = list_get(lista_memorias_aux,i);
//
//					printf("NRO MEMORIA: %i\n", registro->memory_number);
//					printf("IP: %s\n", registro->ip);
//					printf("PUERTO: %i\n\n", registro->puerto);
//
//				}

			}

			cerrarSocket(fdMemoria);

		}



	}

	int u;
	for(u=0;u<list_size(lista_memorias);u++){
		memoria_t* m = list_get(lista_memorias,u);
		free(m->ip);

	}

	list_clean(lista_memorias);

	int p;
	for(p=0;p<list_size(lista_memorias_aux);p++){
		memoria_t* me = list_get(lista_memorias_aux,p);

		bool esMemoria(memoria_t* mem_aux){return mem_aux->memory_number == me->memory_number;}
		memoria_t* memoria_encontrada = list_find(lista_memorias,(void*) esMemoria);

		if(memoria_encontrada == NULL){
			list_add(lista_memorias,me);
		}


	}


	//printf("list_size(lista_memorias): %i\n",list_size(lista_memorias));
	log_info(logger,"Resultado de gossiping");
	int c;
	for(c=0;c<list_size(lista_memorias);c++){
		memoria_t* registro = list_get(lista_memorias,c);

		log_info(logger,"NRO MEMORIA: %i",registro->memory_number);
		log_info(logger,"IP: %s", registro->ip);
		log_info(logger,"PUERTO: %i", registro->puerto);

		//printf("NRO MEMORIA: %i\n", registro->memory_number);
		//printf("IP: %s\n", registro->ip);
		//printf("PUERTO: %i\n\n", registro->puerto);

	}



}


//void gossiping()
//{
//	//Formato en que se recibe la tabla gossiping con dos memorias (NroMemoria-IP-Puerto)
//	//2*1-127.0.0.1-8000/2-127.0.0.1-8002/
//	//printf("hilo gossiping\n");
//	int fdMemoria = nuevoSocket();
//	if (conectarConMemoria(fdMemoria)){ // Conexión con Memoria
//
//		handshake_cliente(fdMemoria, "K");
//
//		int bytes = aplicar_protocolo_enviar(fdMemoria,TABLA_GOSSIPING,NULL);
//		int* head = malloc(INT);
//		char* buffer = aplicar_protocolo_recibir(fdMemoria,head);
//
//		int tam_buffer = strlen(buffer);
//
//		buffer[tam_buffer -1] = '\0';
//
//		char** mensaje  = string_n_split(buffer, 2, "*");
//		int cabeza = atoi(mensaje[0]);
//		char* cuerpo = mensaje[1];
//		char** memorias  = string_n_split(cuerpo, cabeza, "/");
//		free(head);
//		int j;
//
//		for(j=0;j<cabeza;j++){
//			char** partes = string_n_split(memorias[j], 3, "-");
//			char* nro_memoria = partes[0];
//			char* ip = partes[1];
//			char* puerto = partes[2];
//
//			int tam_ip = strlen(ip);
//
//			memoria_t* nodo = malloc(sizeof(memoria_t)) ;
//			nodo->ip = malloc(tam_ip +1);
//
//			strcpy(nodo->ip,ip);
//			nodo->puerto = atoi(puerto);
//			nodo->memory_number = atoi(nro_memoria);
//
//			list_add(lista_memorias,nodo);
//
//		}
//
//		int i;
//		for(i=0;i<list_size(lista_memorias);i++){
//			memoria_t* registro = list_get(lista_memorias,i);
//
////			printf("NRO MEMORIA: %i\n", registro->memory_number);
////			printf("IP: %s\n", registro->ip);
////			printf("PUERTO: %i\n\n", registro->puerto);
//
//		}
//
//		cerrarSocket(fdMemoria);
//
//	}
//
//}

memoria_t* get_memoria(char* consistencia, int keyQuery ){

	memoria_t* mem = NULL;
	if(strcmp(consistencia,"SC") == 0){
		if(list_size(lista_sc) > 0){
			mem = list_get(lista_sc,0);
			return mem;
		}else{
			printf("No hay memoria en el criterio SC\n");
			return mem;
		}
	}

	if(strcmp(consistencia,"SHC") == 0){
		if(list_size(lista_shc) > 0){
			//printf("\nEstoy en la busqueda de la memoria SHC\n");
			int nro_memoria = generarHash( keyQuery ) ;
			//printf("\nA partir del HASH obtengo el indice de la memoria:%d\n", nro_memoria);
			log_info(logger,"A partir del HASH obtengo el indice de la memoria: %d", nro_memoria);
			mem = list_get(lista_shc,nro_memoria);
			return mem;

		}else{
			printf("No hay memoria en el criterio SHC\n");
			return mem;
		}
	}

	if(strcmp(consistencia,"EC") == 0){
		mem = queue_pop(cola_ec);
		queue_push(cola_ec,mem);
		return mem;
	}else{
		printf("No hay memoria en el criterio EC\n");
		return mem;
	}

	return mem;
}


char* get_consistencia_tabla(char* query){

	char* consistencia;
	char** parametros = string_n_split(query, 6, " ");
	char* nombre_tabla = parametros[1];
	//printf("nombre_tabla: %s\n",nombre_tabla);

	//Si la query es un Create entonces aun no existe la tabla.
	//La consistencia la tengo que sacar de la query
	string_to_upper(parametros[0]);
	if(strcmp(parametros[0],COMANDO_CREATE) == 0){

		consistencia = malloc(4);
		strcpy(consistencia,parametros[2]);
		//printf("consistencia: %s\n",consistencia);
		return consistencia;
	}
//	if(strcmp(parametros[0],COMANDO_DESCRIBE) == 0){
//
//		consistencia = malloc(4);
//		memset(consistencia,'\0',4);
//		strcpy(consistencia,parametros[1]);
//		printf("consistencia: %s\n",consistencia);
//		return consistencia;
//	}

	bool esTabla(t_config_MetaData_tabla* tabla){return strcmp(tabla->nombre_tabla,nombre_tabla) == 0;}
	t_config_MetaData_tabla* tabla_buscada = (t_config_MetaData_tabla*)list_find(lista_tablas,(void*) esTabla);
	//printf("list_size(lista_tablas): %i\n",list_size(lista_tablas));
	//printf("tabla->consistency: %s\n",tabla_buscada->consistency);
	consistencia = malloc(4);
	memset(consistencia,'\0',4);
	strcpy(consistencia,tabla_buscada->consistency);

	//printf("consistencia: %s\n",consistencia);
	return consistencia;

}

//Luego que se hace el create se actualiza la metadata en la lista de tablas que tiene el Kernel
void actualizar_metadata_from_create(char* metadata){

	char** datos = string_n_split(metadata,5," ");
	bool esTabla(t_config_MetaData_tabla* tabla){ return strcmp(tabla->nombre_tabla,datos[1]) == 0; }

	t_config_MetaData_tabla* nodo_tabla  = (t_config_MetaData_tabla*) list_find(lista_tablas, (void*) esTabla);

	if(nodo_tabla == NULL){
		t_config_MetaData_tabla* meta_tabla = malloc(sizeof(t_config_MetaData_tabla));
		int tam_nombre = strlen(datos[1]);

		meta_tabla->nombre_tabla = malloc(tam_nombre + 1);
		strcpy(meta_tabla->nombre_tabla,datos[1]);


		meta_tabla->consistency = malloc(4);
		memset(meta_tabla->consistency,'\0',4);
		strcpy(meta_tabla->consistency,datos[2]);


		meta_tabla->partitions = atoi(datos[3]);
		meta_tabla->compaction_time = atoi(datos[4]);

		pthread_mutex_lock(&mutex_lista_tablas);
		list_add(lista_tablas,meta_tabla);
		pthread_mutex_unlock(&mutex_lista_tablas);

	}else{
		strcpy(nodo_tabla->nombre_tabla,datos[1]);
		strcpy(nodo_tabla->consistency,datos[2]);
		nodo_tabla->partitions = atoi(datos[3]);
		nodo_tabla->compaction_time = atoi(datos[4]);
	}
}

void ejecutar_comando_create(char* query, memoria_t* mem){
	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria

		handshake_cliente(socket, "K");
		aplicar_protocolo_enviar(socket,CREATE,query);


		char* resultado = aplicar_protocolo_recibir(socket,&head);

		actualizar_metadata_from_create(query);
		//printf("head: %i\n",head);
		//printf("resultado: %s\n",resultado);
		free(resultado);

	}

	cerrarSocket(socket);
}

void atender_memoria_full_select(int socket){

	//Este envío de journal va hacer que la memoria haga dos cosas
	//1) Ejecutar Journal
	//2)Ejecutar el select mandado anteriormente
	int bytes_enviados = aplicar_protocolo_enviar(socket,JOURNAL,COMANDO_JOURNAL);

	//printf("bytes_enviados: %i\n",bytes_enviados);
	int head;
	char* resultado = aplicar_protocolo_recibir(socket,&head);
	//printf("resultado: %s\n",resultado);


	free(resultado);
}
void ejecutar_comando_select(char* query, memoria_t* mem){

	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria
		handshake_cliente(socket, "K");
		int bytes_enviados = aplicar_protocolo_enviar(socket,SELECT,query);
		//printf("bytes_enviados: %i\n",bytes_enviados);
		char* resultado = aplicar_protocolo_recibir(socket,&head);
		//printf("resultado final: %s\n",resultado);
		if(strcmp(resultado,NO_EXISTE_LA_KEY) == 0){//Si no existe la key en TODO el sistema
			printf("No existe la key en el LFS\n");
		}

		if(strcmp(resultado,NO_EXISTE_LA_TABLA) == 0){//Si no existe la tabla en el FS
			printf("No existe la tabla en el LFS \n");
			return;
		}

		if(strcmp(resultado,MEMORIA_FULL) == 0){
			atender_memoria_full_select(socket);
		}

		printf("resultado: %s\n",resultado);

		free(resultado);

	}

	cerrarSocket(socket);
}

void ejecutar_comando_insert(char* query, memoria_t* mem){
//	printf("estoy en ejecutar_comando_insert\n");
//	printf("query final : %s\n",query);
//	printf("mem->puerto : %i\n",mem->puerto);
//	printf("mem->ip : %s\n",mem->ip);
	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria

		handshake_cliente(socket, "K");
		int bytes_enviados = aplicar_protocolo_enviar(socket,INSERT,query);
		//printf("bytes_enviados: %i\n",bytes_enviados);
		char* resultado = aplicar_protocolo_recibir(socket,&head);
		//printf("head: %i\n",head);
		//printf("resultado: %s\n",resultado);

		if(strcmp(resultado,MEMORIA_FULL) == 0){
			int bytes_enviados = aplicar_protocolo_enviar(socket,JOURNAL,COMANDO_JOURNAL);
			//printf("bytes_enviados: %i\n",bytes_enviados);
			char* resultado = aplicar_protocolo_recibir(socket,&head);
			int bytes_enviados2 = aplicar_protocolo_enviar(socket,INSERT,query);
			char* resultado2= aplicar_protocolo_recibir(socket,&head);
			//printf("resultado: %s\n",resultado);
			free(resultado2);
		}
		free(resultado);

	}

	cerrarSocket(socket);
}

void actualizar_metadata(char* metadata){
	char** datos = string_n_split(metadata,4," ");
	bool esTabla(t_config_MetaData_tabla* tabla){ return strcmp(tabla->nombre_tabla,datos[0]) == 0; }

	t_config_MetaData_tabla* nodo_tabla  = (t_config_MetaData_tabla*) list_find(lista_tablas, (void*) esTabla);

	if(nodo_tabla == NULL){
		t_config_MetaData_tabla* meta_tabla = malloc(sizeof(t_config_MetaData_tabla));
		int tam_nombre = strlen(datos[0]);

		meta_tabla->nombre_tabla = malloc(tam_nombre + 1);
		strcpy(meta_tabla->nombre_tabla,datos[0]);


		meta_tabla->consistency = malloc(4);
		memset(meta_tabla->consistency,'\0',4);
		strcpy(meta_tabla->consistency,datos[1]);


		meta_tabla->partitions = atoi(datos[2]);
		meta_tabla->compaction_time = atoi(datos[3]);

		pthread_mutex_lock(&mutex_lista_tablas);
		list_add(lista_tablas,meta_tabla);
		pthread_mutex_unlock(&mutex_lista_tablas);

	}else{
		strcpy(nodo_tabla->nombre_tabla,datos[0]);
		strcpy(nodo_tabla->consistency,datos[1]);
		nodo_tabla->partitions = atoi(datos[2]);
		nodo_tabla->compaction_time = atoi(datos[3]);
	}
}

void ejecutar_comando_drop(char* query, memoria_t* mem){

	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria

		handshake_cliente(socket, "K");
		int bytes_enviados = aplicar_protocolo_enviar(socket,DROP,query);
		//printf("bytes_enviados: %i\n",bytes_enviados);
		char* resultado = aplicar_protocolo_recibir(socket,&head);
		//printf("head: %i\n",head);
		//printf("resultado: %s\n",resultado);

		free(resultado);

	}

	cerrarSocket(socket);
}

void ejecutar_comando_describe(char* query, memoria_t* mem){

	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria

		handshake_cliente(socket, "K");
		int bytes_enviados = aplicar_protocolo_enviar(socket,DESCRIBE,query);
		//printf("bytes_enviados: %i\n",bytes_enviados);
		char* resultado = aplicar_protocolo_recibir(socket,&head);
		//printf("head: %i\n",head);
		//printf("resultado: %s\n",resultado);
		actualizar_metadata(resultado);
		free(resultado);

	}

	cerrarSocket(socket);
}


unsigned long long obtener_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = ((unsigned long long)tv.tv_sec*1e3) + ((unsigned long long)tv.tv_usec/1000);
	return timestamp;
}


void agregar_metricas_select(unsigned long long duracion,int nro_memoria){

	metricas->cant_reads = metricas->cant_reads + 1;
	metricas->tiempo_reads = metricas->tiempo_reads + duracion;
	metricas->cant_inserts_select_total = metricas->cant_inserts_select_total + 1;
	bool esta_mem(memorias_metrics_t* memori){return memori->nro_memoria == nro_memoria;}
	memorias_metrics_t* mem_buscada = (memorias_metrics_t*)list_find(lista_tablas,(void*) esta_mem);
	if(mem_buscada == NULL){
		memorias_metrics_t* mem1 = malloc(sizeof(memorias_metrics_t));
		mem1->nro_memoria = nro_memoria;
		mem1->cant_inserts_y_selects = 1;
		list_add(metricas->memorias_metrics,mem1);
	}else{
		mem_buscada->cant_inserts_y_selects = mem_buscada->cant_inserts_y_selects + 1;
	}
}


void agregar_metricas_insert(unsigned long long duracion,int nro_memoria){

	metricas->cant_writes = metricas->cant_writes + 1;
	metricas->tiempo_writes = metricas->tiempo_writes + duracion;
	metricas->cant_inserts_select_total = metricas->cant_inserts_select_total + 1;
	bool esta_mem(memorias_metrics_t* memori){return memori->nro_memoria == nro_memoria;}
	memorias_metrics_t* mem_buscada = (memorias_metrics_t*)list_find(lista_tablas,(void*) esta_mem);
	if(mem_buscada == NULL){
		memorias_metrics_t* mem1 = malloc(sizeof(memorias_metrics_t));
		mem1->nro_memoria = nro_memoria;
		mem1->cant_inserts_y_selects = 1;
		list_add(metricas->memorias_metrics,mem1);
	}else{
		mem_buscada->cant_inserts_y_selects = mem_buscada->cant_inserts_y_selects + 1;
	}
}


void atender_pedido(char* query, memoria_t* mem){


	char** parametros = string_n_split(query, 6, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_CREATE)) {
		ejecutar_comando_create(query,mem);


	} else if (string_equals_ignore_case(parametros[0], COMANDO_SELECT)) {
		unsigned long long time_inicio = obtener_timestamp();
		ejecutar_comando_select(query,mem);
		unsigned long long time_fin = obtener_timestamp();

		unsigned long long duracion = (time_fin - time_inicio);

		pthread_mutex_lock(&mutex_metricas);
		agregar_metricas_select( duracion, mem->memory_number);
		pthread_mutex_unlock(&mutex_metricas);


	} else if (string_equals_ignore_case(parametros[0], COMANDO_INSERT)) {

		unsigned long long time_inicio = obtener_timestamp();

		ejecutar_comando_insert(query,mem);

		unsigned long long time_fin = obtener_timestamp();

		unsigned long long duracion = (time_fin - time_inicio);

		pthread_mutex_lock(&mutex_metricas);
		agregar_metricas_insert(duracion, mem->memory_number);
		pthread_mutex_unlock(&mutex_metricas);

	}  else if (string_equals_ignore_case(parametros[0], COMANDO_DESCRIBE)) {
		ejecutar_comando_describe(query,mem);

	} else if (string_equals_ignore_case(parametros[0], COMANDO_DROP)) {
		ejecutar_comando_drop(query,mem);

	}



}

int cant_instruc_to_ejecutar(pcb_t* pcb){
	int cant_instruc_faltan = pcb->cant_instrucciones - pcb->ip;

	if(cant_instruc_faltan < configFile->quantum ){
		return cant_instruc_faltan;
	}else{
		return configFile->quantum;
	}
}
void actualizar_ip(pcb_t* pcb){
	//Ya ejecuto una instruccion ahora tengo que actualizar la IP

	if((pcb->cant_instrucciones - pcb->ip) == 1){
		//No se actualiza porque ya ejecuto la ultima instruccion
		//Lo tengo que agregar a la cola de EXIT
	}else{
		 pcb->ip =  pcb->ip + 1;
	}
}

int atender_si_es_un_describe_full(char* query){
	char** campos = string_n_split(query, 6, " ");
	if(strcmp(campos[0],COMANDO_DESCRIBE) == 0){
		printf("campos[1]: %s\n",campos[1]);
		if(campos[1] == NULL){//es un describe full
			printf("es un describe global\n");
			actualizar_info_tablas();
			return 1;
		}
	}
	return 0;
}
void ejecutar_comando_journal(memoria_t* mem){
	int socket = nuevoSocket();
	int head;
	if (conectar_to_memoria(socket,mem->puerto, mem->ip)){ // Conexión con Memoria

		handshake_cliente(socket, "K");
		int bytes_enviados = aplicar_protocolo_enviar(socket,JOURNAL,COMANDO_JOURNAL);
		//printf("bytes_enviados: %i\n",bytes_enviados);
		char* resultado = aplicar_protocolo_recibir(socket,&head);
		//printf("head: %i\n",head);
		//printf("resultado: %s\n",resultado);

		free(resultado);

	}

	cerrarSocket(socket);
}

void crear_vector_memorias(){

}
int memoria_atendida(int *memorias,int valor,int cant_mem_max){
	int z;
	for(z=0;z<cant_mem_max;z++){
		if(memorias[z] == valor){
			return z;
		}
	}
	return -1;
}
void init_vector_aux(int *memorias,int cant_mem_max){

	int z;
	for(z=0;z<cant_mem_max;z++){
		memorias[z] = -1;
	}
}

void add_valor_a_vector_aux(int *memorias,int valor,int cant_mem_max){
	int z;
	for(z=0;z<cant_mem_max;z++){
		if(memorias[z] == -1){
			memorias[z] = valor;
			return;
		}
	}
}
int atender_si_es_un_journal(char* query){
	//Tengo que enviar journal a cada una de las memorias que tenga
	//asociado a los criterios! No a todas las memorias sino solo a aquella
	//que estan en algun criterio
	int cant_mem_max = 50;
	int memorias[cant_mem_max];

	init_vector_aux(memorias,cant_mem_max);

	//printf("query::: %s\n",query);
	if(strcmp(query,COMANDO_JOURNAL) == 0){
		//printf("es un journal\n");
		int i;
		for(i=0;i<queue_size(cola_ec);i++){
			memoria_t* mem = list_get(cola_ec->elements,i);
			//printf("NRO MEMORIA: %i\n", mem->memory_number);
			//printf("IP: %s\n", mem->ip);
			//printf("PUERTO: %i\n\n", mem->puerto);

			ejecutar_comando_journal(mem);

			 add_valor_a_vector_aux(memorias, mem->memory_number, cant_mem_max);

		}
		int j;
		for(j=0;j<list_size(lista_shc);j++){
			memoria_t* mem = list_get(lista_shc,j);
			if(memoria_atendida(memorias,mem->memory_number,cant_mem_max) < 0){
				//No fue atendida
				ejecutar_comando_journal(mem);
				add_valor_a_vector_aux(memorias, mem->memory_number, cant_mem_max);
			}
		}

		int p;
		for(p=0;p<list_size(lista_sc);p++){
			memoria_t* mem = list_get(lista_sc,p);
			if(memoria_atendida(memorias,mem->memory_number, cant_mem_max) < 0){
				//No fue atendida
				ejecutar_comando_journal(mem);
				add_valor_a_vector_aux(memorias, mem->memory_number, cant_mem_max);
			}
		}


		return 1;
	}



	return 0;
}
void consumidor()
{

	while(TRUE){
		//Saco un script de la cola de scripts y lo atiendo
		sem_wait(&contador_cola_listos);
		pthread_mutex_lock(&mutex_cola_listos);

		pcb_t* pcb  = queue_pop(cola_listos);
		pthread_mutex_unlock(&mutex_cola_listos);
		//printf("atiendo a un nuevo pcb");
//		int z;
//		for(z=0;z<pcb->cant_instrucciones;z++){
//
//			printf("iiiiiiinstruc: %i =  %s\n",z,pcb->instruciones[z]);
//		}

		int cantidad_instrucciones = cant_instruc_to_ejecutar(pcb);
		//printf("cantidad_instrucciones pcb: %i\n",cantidad_instrucciones);
		int i;
		for(i=0;i<cantidad_instrucciones;i++){
			//obtengo la siguiente query
			//printf("pcb->ip : %i\n",pcb->ip);
			int pc = pcb->ip;
			char* query = pcb->instruciones[pc];
			//printf("Instruccion: %s\n",query);
			log_info(logger,"Instruccion: %s\n",query);
			int es_describe_full = atender_si_es_un_describe_full(query);
			int es_journal = atender_si_es_un_journal(query);
			if((es_describe_full == 0) & (es_journal == 0)){
				char* consistencia = get_consistencia_tabla(query);
				char** parametros = string_n_split(query, 6, " ");
				memoria_t* mem =  get_memoria(consistencia,atoi(parametros[2]));
				if(mem == NULL){
					printf("No hay memoria en el criterio requerido\n");
					return ;
				}

				free(consistencia);
				atender_pedido(query, mem);
			}

			pcb->ip = pcb->ip + 1;
		  }

		 //printf("pcb->ip final: %i\n",pcb->ip);
		 //printf("pcb->cant_instrucciones: %i\n",pcb->cant_instrucciones);
			if(pcb->ip < pcb->cant_instrucciones){
				//printf("El script vuelve a la cola de listos\n");
				pthread_mutex_lock(&mutex_cola_listos);
				queue_push(cola_listos,pcb);
				pthread_mutex_unlock(&mutex_cola_listos);
				sem_post(&contador_cola_listos);
			}else{
				//printf("Se termino de ejecutar el script\n");
			}






	}
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
		log_info(logger, "Handshake recibido: %s", buff);

		free(buff);
		enviarPorSocket(sockClienteDe, mensaje, HANDSHAKE_SIZE);
	}
}


int conectarConMemoria_gossiping(int fdMem, char* ip, int puerto) {

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

int conectarConMemoria(int fdMem) {

	//fdMem = nuevoSocket();
	int conexion = conectarSocket(fdMem, configFile->ipMemoria, configFile->puertoMemoria);

	if(conexion == ERROR){
		log_error(logger, "Falló conexión con Memoria.");

		return FALSE;
	}
	else{
		return TRUE;
	}

}

int conectar_to_memoria(int fdMem,int puerto, char* ip) {

	int conexion = conectarSocket(fdMem, ip, puerto);

	if(conexion == ERROR){
		log_error(logger, "Falló conexión con Memoria.");

		return FALSE;
	}
	else{
		return TRUE;
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
		if (ejecutar_comando(comando) == 0) {
			char* comando_listo = comando_preparado(comando);
			log_info(logger,"El comando %s fue ejecutado con exito", comando_listo);
		}

	}

	free(comando);


}


void crearLoggerKernel() {
	char * archivoLog = strdup("KERNEL_LOG.log");
	logger = log_create(LOG_PATH, archivoLog, FALSE, LOG_LEVEL_INFO);
	free(archivoLog);
	archivoLog = NULL;
}



void load_config_file(char *ruta) {

	//configFile = reservarMemoria(sizeof(archivoConfigKernel));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por re-setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "KERNEL: Obteniendo nuevos valores del Archivo de Configuracion..");

		if (config_has_property(config, "IP_MEMORIA")) {
			configFile->ipMemoria = strdup(
											config_get_string_value(config, "IP_MEMORIA"));
									log_info(logger,
											"Se encontró y cargó el contenido de IP_MEMORIA a la estructura de KERNEL. Valor: %s",configFile->ipMemoria);

								} else {
									log_error(logger,
											"El archivo de configuracion no contiene el IP_MEMORIA con el parametro IP_MEMORIA");
				}

		if (config_has_property(config, "PUERTO_MEMORIA")) {
			configFile->puertoMemoria = config_get_int_value(config,
							"PUERTO_MEMORIA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_MEMORIA a la estrcutra de KERNEL. Valor: %d",configFile->puertoMemoria);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO_MEMORIA  con el parametro PUERTO_MEMORIA");

		}

		if (config_has_property(config, "QUANTUM")) {
			configFile->quantum = config_get_int_value(config,
									"QUANTUM");
							log_info(logger,
									"Se encontró y cargó el contido del QUANTUM a la estrcutra de KERNEL. Valor: %d",configFile->quantum);
						} else {

							log_error(logger,
									"El archivo de configuracion no contiene el QUANTUM  con el parametro QUANTUM");

		}

		if (config_has_property(config, "MULTIPROCESAMIENTO")) {
			configFile->multiprocesamiento = config_get_int_value(config,
											"MULTIPROCESAMIENTO");
									log_info(logger,
											"Se encontró y cargó el contido del MULTIPROCESAMIENTO a la estrcutra de KERNEL. Valor: %d",configFile->multiprocesamiento);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el MULTIPROCESAMIENTO  con el parametro MULTIPROCESAMIENTO");

				}

		if (config_has_property(config, "METADATA_REFRESH")) {
			configFile->refreshMetadata = config_get_int_value(config,
													"METADATA_REFRESH");
											log_info(logger,
													"Se encontró y cargó el contido del METADATA_REFRESH a la estrcutra de KERNEL. Valor: %d",configFile->refreshMetadata);
										} else {

											log_error(logger,
													"El archivo de configuracion no contiene el METADATA_REFRESH  con el parametro METADATA_REFRESH");

		}

		if (config_has_property(config, "SLEEP_EJECUCION")) {
			configFile->retardoCicloEjecucion = config_get_int_value(config,
															"SLEEP_EJECUCION");
													log_info(logger,
															"Se encontró y cargó el contido del SLEEP_EJECUCION a la estrcutra de KERNEL. Valor: %d",configFile->retardoCicloEjecucion);
												} else {

													log_error(logger,
															"El archivo de configuracion no contiene el SLEEP_EJECUCION  con el parametro SLEEP_EJECUCION");

				}

		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"Se ha obtenido correctamente los nuevos valores del archivo de configuracion");

	}
}

void leerArchivoDeConfiguracion(char *ruta) {

	configFile = reservarMemoria(sizeof(archivoConfigKernel));
	t_config *config;
	config = config_create(ruta);
	log_info(logger, "Por setear los valores del archivo de configuracion");
	if (config != NULL) {
		log_info(logger, "KERNEL: Leyendo Archivo de Configuracion..");

		if (config_has_property(config, "IP_MEMORIA")) {
			configFile->ipMemoria = strdup(
											config_get_string_value(config, "IP_MEMORIA"));
									log_info(logger,
											"Se encontró y cargó el contenido de IP_MEMORIA a la estructura de KERNEL. Valor: %s",configFile->ipMemoria);

								} else {
									log_error(logger,
											"El archivo de configuracion no contiene el IP_MEMORIA con el parametro IP_MEMORIA");
				}

		if (config_has_property(config, "PUERTO_MEMORIA")) {
			configFile->puertoMemoria = config_get_int_value(config,
							"PUERTO_MEMORIA");
					log_info(logger,
							"Se encontró y cargó el contido del PUERTO_MEMORIA a la estrcutra de KERNEL. Valor: %d",configFile->puertoMemoria);
				} else {

					log_error(logger,
							"El archivo de configuracion no contiene el PUERTO_MEMORIA  con el parametro PUERTO_MEMORIA");

		}

		if (config_has_property(config, "QUANTUM")) {
			configFile->quantum = config_get_int_value(config,
									"QUANTUM");
							log_info(logger,
									"Se encontró y cargó el contido del QUANTUM a la estrcutra de KERNEL. Valor: %d",configFile->quantum);
						} else {

							log_error(logger,
									"El archivo de configuracion no contiene el QUANTUM  con el parametro QUANTUM");

		}

		if (config_has_property(config, "MULTIPROCESAMIENTO")) {
			configFile->multiprocesamiento = config_get_int_value(config,
											"MULTIPROCESAMIENTO");
									log_info(logger,
											"Se encontró y cargó el contido del MULTIPROCESAMIENTO a la estrcutra de KERNEL. Valor: %d",configFile->multiprocesamiento);
								} else {

									log_error(logger,
											"El archivo de configuracion no contiene el MULTIPROCESAMIENTO  con el parametro MULTIPROCESAMIENTO");

				}

		if (config_has_property(config, "METADATA_REFRESH")) {
			configFile->refreshMetadata = config_get_int_value(config,
													"METADATA_REFRESH");
											log_info(logger,
													"Se encontró y cargó el contido del METADATA_REFRESH a la estrcutra de KERNEL. Valor: %d",configFile->refreshMetadata);
										} else {

											log_error(logger,
													"El archivo de configuracion no contiene el METADATA_REFRESH  con el parametro METADATA_REFRESH");

		}

		if (config_has_property(config, "SLEEP_EJECUCION")) {
			configFile->retardoCicloEjecucion = config_get_int_value(config,
															"SLEEP_EJECUCION");
													log_info(logger,
															"Se encontró y cargó el contido del SLEEP_EJECUCION a la estrcutra de KERNEL. Valor: %d",configFile->retardoCicloEjecucion);
												} else {

													log_error(logger,
															"El archivo de configuracion no contiene el SLEEP_EJECUCION  con el parametro SLEEP_EJECUCION");

				}

		if (config_has_property(config, "RETARDO_GOSSIPING")) {
					configFile->retardoGossiping = config_get_int_value(config,
																	"RETARDO_GOSSIPING");
															log_info(logger,
																	"Se encontró y cargó el contido del RETARDO_GOSSIPING a la estrcutra de KERNEL. Valor: %d",configFile->retardoGossiping);
														} else {

															log_error(logger,
																	"El archivo de configuracion no contiene el RETARDO_GOSSIPING  con el parametro RETARDO_GOSSIPING");

						}

		config_destroy(config); // Libero la estructura archivoConfig

		log_info(logger,"El archivo de configuración ha sido leído correctamente");

	}
}


int generarHash(int key ) {
	return key % list_size(lista_shc) ;
}
