/*
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */


#include "funcionesLfs.h"

void* reservarMemoria(int size) {
	void *puntero = malloc(size);
	if (puntero == NULL) {
		fprintf(stderr,
				"Lo imposible sucedió. Error al reservar %d bytes de memoria\n",
				size);
		exit(ERROR);
	}
	return puntero;
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

// DESEREAILZAR: Del buffer, al mensaje listo para recibir
void * deserealizar(int head, void * buffer, int tamanio){

	void * mensaje = NULL;

	switch(head){
	// CASE 1: El mensaje es un texto (char*)
	case TAMANIO_VALUE: case DESCRIBE: case CREATE: case SELECT: case JOURNAL: case DROP:{
		mensaje = malloc(tamanio);
		memcpy(mensaje, buffer, tamanio);
		break;
	}

 }
	return mensaje;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)



void remueve_salto_de_linea(char* salida, char* texto) {
	strncpy(salida, texto, strlen(texto) - 1);
}
char* comando_preparado(char* comando) {
	char* comando_listo = calloc(1, strlen(comando));
	remueve_salto_de_linea(comando_listo, comando);
	return comando_listo;
}

int existe_tabla(char* nombre_tabla) {

	int i = 0;
	tabla_t *aux = NULL;
	for (; i < list_size(lista_tablas); i++){
		aux = (tabla_t*) list_get(lista_tablas, i);


		if(!strcmp(aux->nombre_tabla, nombre_tabla)){
			return i;
		}

	}
	return ERROR; // no se encontró la tabla
}

//int  ejecutar_comando_insert(char* nombre_tabla, int key, char* valor, unsigned long timestamp)
//{
//
//	int index_tabla;
//	index_tabla = existe_tabla(nombre_tabla);
//	if(index_tabla >= 0){//La tabla SI existe
//		agregar_registro( index_tabla,  key,  valor,  timestamp);
//	}else{//La tabla no existe y la tenemos que agregar.
//		log_error(logger,"La tabla no existe");
//		return EXIT_FAILURE;
//	}
//
//	return EXIT_SUCCESS;
//
//}

//Devuelve 1 si ejecuto ok
//-1 si la tabla no existe
int  ejecutar_comando_insert(char* nombre_tabla, int key, char* valor, unsigned long long timestamp){

	if(timestamp == NULL){
		timestamp = obtener_timestamp();
	}
	//printf("timestamp: %llu\n",timestamp);


	int existe = existe_tabla_en_disco(nombre_tabla);
	if(existe < 0){//La tabla si existe
		int index_tabla;
		index_tabla = existe_tabla(nombre_tabla);
		if(index_tabla >= 0){//La tabla SI existe
			agregar_registro( index_tabla,  key,  valor,  timestamp);
		}else{//La tabla no existe en memoria la tenemos que agregar.
			t_config_MetaData_tabla* info = leer_metaData_tabla(nombre_tabla);
			agregar_tabla(info->nombre_tabla,info->consistency,info->partitions,info->compaction_time);
			index_tabla = existe_tabla(nombre_tabla);
			agregar_registro( index_tabla,  key,  valor,  timestamp);

			free(info);
		}
		return 1;
	}else{
		log_error(logger," ERROR. La tabla no existe en el File System.");
		return -1;
	}


}



int crear_directorioTabla(char* nombre_tabla,char* dir)
{
	char* punto_montaje_tables = path_punto_montaje_tables();

	char * directorioTabla = (char*) malloc(1 + string_length(punto_montaje_tables)+string_length(nombre_tabla)) ;
	strcpy(directorioTabla,punto_montaje_tables);
	strcat(directorioTabla, nombre_tabla);

	if(mkdir(directorioTabla,S_IRWXU)==-1)//crea el directorio de la tabla
	{
		log_error(logger," ERROR. La tabla que intenta crear ya existe.");
		return -1;
	}
	free(directorioTabla);
	free(punto_montaje_tables);
	return 1;
}


int crear_metadata(char* dir, char* nombre_tabla,char* consistency, int cant_partitions, int compaction_time)
{
	FILE *metadata;
	char * dirMetadata = (char *) malloc(1 + strlen(dir) + strlen(nombre_tabla)+ strlen("/Metadata.bin") );

	strcpy(dirMetadata,dir);
	strcat(dirMetadata,nombre_tabla);
	strcat(dirMetadata, "/Metadata.bin");

	metadata = fopen (dirMetadata, "wb");
	if (metadata==NULL)
	{
	   perror("No se pudo crear el archivo metadata");
	   return -1;
	}else  //si crea el metadata correctamente, lo rellena con los datos que le pasaron
	{
		fprintf (metadata,"CONSISTENCY=%s\nPARTITIONS=%d\nCOMPACTION_TIME=%d\n",consistency,cant_partitions,compaction_time);
	}
	fclose(metadata);
	free(dirMetadata);
	return 1;
}

int crear_particiones(char* dir,char* nombre_tabla,int cant_partitions)
{
	// Creamos las particiones
	int i;
	for(i = 0; i< cant_partitions;i++)
	{
		FILE *partition;

		char * dirPartition = (char *) malloc(1 + strlen(dir) + strlen(nombre_tabla)+ strlen("/")+ strlen(string_itoa(i)) + strlen(".bin") );
		strcpy(dirPartition,dir);
		strcat(dirPartition, nombre_tabla);
		strcat(dirPartition, "/");
		strcat(dirPartition, string_itoa(i));
		strcat(dirPartition, ".bin");

		partition = fopen (dirPartition, "wb");
		if (partition==NULL)
		{
		   perror("No se pudo crear la partición");
		   printf("%d",i);
		   return -1;
		}
		else  //si crea laparticion correctamente, lo rellena con los datos que le pasaron
		{
			fprintf (partition,"SIZE=\nBLOCKS=[]");
		}
		fclose(partition);
		free(dirPartition);
	}
	return 1;
}

int existe_tabla_en_disco(char* nom_tabla_buscada){
	DIR *dp;
	struct dirent *ep;
	string_to_upper(nom_tabla_buscada);
	char* path_tables = path_punto_montaje_tables();
	dp = opendir (path_tables);
	if (dp != NULL)
	{
		while ((ep = readdir (dp))!= NULL)
		{
			if(strcmp(ep->d_name,".")!= 0 && strcmp(ep->d_name,"..") != 0)
			{
				  if (ep->d_type == DT_DIR)
				  {
					char* nom_tabla;
					nom_tabla = (char*) malloc (1 + string_length(ep->d_name));
					strcpy(nom_tabla,ep->d_name);
					string_to_upper(nom_tabla);

					if(!strcmp(nom_tabla,nom_tabla_buscada)){
						return -1;
					}
				  }
			}
		}


	}
	(void) closedir (dp);
	free(path_tables);
	return 1;
}

int crear_tabla_fs (char* nombre_tabla,char* consistency, int cant_partitions, int compaction_time)
{

	int estado = 0;
	//Una vez creado tables, crea la nueva tabla con los archivos correspondientes

	char* punto_montaje_tables = path_punto_montaje_tables();

	do{
		switch(estado) //maquina de estados para la creación de tablas
		{

			case 0:
						if (crear_directorioTabla(nombre_tabla,punto_montaje_tables) == 1)
						{
							estado = 1;
							break;
						}
						else
						{
							return -1;
						}
			case 1:
						if (crear_metadata(punto_montaje_tables,nombre_tabla,consistency,cant_partitions, compaction_time) == 1)
						{
							estado = 2;
							break;
						}
						else
						{
							return -2;
						}
			case 2:
						if (crear_particiones(punto_montaje_tables,nombre_tabla,cant_partitions) == 1)
						{
							estado = 3;
							break;
						}
						else
						{
							return -3;
						}
			case 3:
						if (agregar_tabla( nombre_tabla, consistency,  cant_partitions, compaction_time )==1)
						{
							estado=4;
							break;
						} else
						{
							return -4;
						}
			default: break;

		}

	}while (estado < 4);

	free(punto_montaje_tables);
	return 1;
}

//
//
//void ejecutar_comando_create_para_memoria(char* query, int sockMemoria){
//
//	char* rta;
//	char **consulta;
//	consulta = string_n_split(query, 5, "|");
//
//
//
//	int existe;
//	existe = existe_tabla_en_disco(consulta[1]);
//	if(existe > 0)
//	{//La tabla no existe y la tenemos que agregar.
//
//		int result = crear_tabla_fs(consulta[1],consulta[2], atoi(consulta[3]), atoi(consulta[4]));
//		rta = string_itoa(result);
//	}else{///La tabla SI existe
//
//		rta = string_itoa(-5);
//		log_error(logger," ERROR. La tabla que intenta crear ya existe.");
//	}
//
//
//	int bytes_enviados = aplicar_protocolo_enviar(sockMemoria,DESCRIBE,rta);
//
//
//}



void ejecutar_comando_create_para_memoria(char* query, int sockMemoria){

	//printf("query 1: %s\n",query);
	char* rta;
	char **consulta;
	consulta = string_n_split(query, 5, " ");



	int existe;
	existe = existe_tabla_en_disco(consulta[1]);
	if(existe > 0)
	{//La tabla no existe y la tenemos que agregar.

		int result = crear_tabla_fs(consulta[1],consulta[2], atoi(consulta[3]), atoi(consulta[4]));
		rta = string_itoa(result);
	}else{///La tabla SI existe

		rta = string_itoa(-5);
		log_error(logger," ERROR. La tabla que intenta crear ya existe.");
	}

	log_info(logger, "rta: %s\n",rta);

	//printf("rta: %s\n",rta);
	int bytes_enviados = aplicar_protocolo_enviar(sockMemoria,CREATE,rta);
	//printf("bytes_enviados: %i\n",bytes_enviados);

}

void renombrar_temporales(char* nom_tabla){
	char* path_tabla = path_directorio_tabla(nom_tabla);
	struct dirent *ep;
	DIR *dp;
	char* ruta = string_new();
	ruta = string_duplicate(path_tabla);

	dp = opendir (path_tabla);

	if (dp != NULL){
			 while ((ep = readdir (dp))!= NULL){

				 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
					 //Descarto el . y los ..
					 printf("%s\n",ep->d_name);
					 char **nombre_archivo = string_split(ep->d_name, ".");

					 if(strcmp(nombre_archivo[1],FORMATO_TMP) == 0 ){
						char* nom_viejo = string_duplicate(ruta);
						string_append(&nom_viejo, "/");
						string_append(&nom_viejo, ep->d_name);

						//printf("nom_viejo: %s\n",nom_viejo);
						char* nom_nuevo = string_duplicate(ruta);
						string_append(&nom_nuevo, "/");
						string_append(&nom_nuevo, nombre_archivo[0]);
						string_append(&nom_nuevo, ".");
						string_append(&nom_nuevo, FORMATO_TMP_C);

						//printf("nom_nuevo: %s\n",nom_nuevo);


						rename(nom_viejo, nom_nuevo);

						free(nom_viejo);
						free(nom_nuevo);
					 }
				 }
			 }
	}

	free(ruta);
	(void) closedir (dp);
	free(path_tabla);



}


short getTamanioArchivo(char * nombreArchivo, int* cant_instruc){


	char* nombreScript = nombreArchivo;
	FILE *archivoScript = fopen(nombreScript, "r");
	if (archivoScript == NULL)
	{
		printf("Error de apertura en script :%s\n\n",nombreScript);
		fclose(archivoScript);
		return 0;
	}


	int cant_instrucciones= 0;
	short i = 0;
	while (!feof(archivoScript))
	{
		//printf("caracter: %i\n",fgetc(archivoScript));
		if(fgetc(archivoScript) == 10){// el nro 10 represeta el '\n' fin de linea
			cant_instrucciones+= 1;
		}

		i++;
	}
	//cant_instrucciones+= 1; //la ultima instruccion no tiene '\n' si no que tiene -1

	*cant_instruc = cant_instrucciones;

	fclose(archivoScript);
	return i;
}

char* leer_bloque(char* path_archivo,int* cant_lineas){

	char *archivo;

	/*OBTENGO EL TAMAÑO DEL ARCHIVO DE SCRIPT */
	int tamanioArchivo = 0;
	if (!(tamanioArchivo = getTamanioArchivo(path_archivo, cant_lineas) -1))
			return EXIT_FAILURE;

	//printf("El tamanio archivo es %i\n",tamanioArchivo);

	/*PASO EL CONTENIDO DEL ARCHIVO  A UN STRING*/
	archivo = get_content_archivo(path_archivo, tamanioArchivo);

	if (!strcmp(archivo, "\0"))
		return NULL;


	//archivo[tamanioArchivo-1]='\0';
	archivo[tamanioArchivo]='\0';
	//printf("ARCHIVO: %s\n", archivo);

	return archivo;

}


char* subir_archivo_to_memoria(char* path){

	//printf("path bloque: %s\n",path);
	bin_tmp_t* file = (bin_tmp_t*) leer_bin_tmp_tmpc(path);
	//printf("file->size: %i\n",file->size);
	//printf("file->bloques: %s\n",file->bloques);
	quitar_corchetes(file);
	//printf("file->bloques: %s\n", file->bloques);
	char** bloques = string_n_split(file->bloques, CANT_MAX_BLOQUES_POR_PARTICION, ",");
	char* ptr_aux = NULL;
	char* ptr = NULL;
	int j = 0;
	int tamanio_total = 0;

	while(bloques[j] != NULL){
		char* ruta_bloque = path_bloque(bloques[j]);

		int* cant_registros = malloc(INT);
		char* archivo_bloque = leer_bloque(ruta_bloque,cant_registros);
		//printf("archivo_bloque: %s\n",archivo_bloque );
		//printf("cant_registros: %i\n",*cant_registros);

		//int tamanio = strlen(archivo_bloque) + 1;
		int tamanio = strlen(archivo_bloque);
		//printf("tamanio: %i\n",tamanio);

		tamanio_total += tamanio;
		//printf("tamanio_total: %i\n",tamanio_total);

		ptr_aux = realloc(ptr,tamanio_total);
		ptr = ptr_aux;
		int desplazamiento = tamanio_total - tamanio;

		memcpy(ptr_aux + desplazamiento,archivo_bloque,tamanio);
//		int i;
//		for(i=0;i< tamanio_total;i++){
//			printf("ptr_aux %i : %c\n",i,ptr_aux[i]);
//		}

		free(cant_registros);


		j++;
	}
	/****hago esto para que no rompa al hacer el printf******/
	ptr_aux = realloc(ptr,tamanio_total + 1);
	ptr = ptr_aux;
	ptr[tamanio_total] = '\0';
	//printf("ptr_aux: %s\n",ptr_aux);
	//*********************************/

	return ptr_aux;

}


void agregarlos_to_lista(t_list* lista_registros, char* registros){

	char** regs = string_split(registros,"\n");
	//printf("registros: %s\n",registros);

	int i=0;
	while(regs[i] != NULL){

		//printf("regs[i]: %s\n",regs[i]);
		char** campos = string_n_split(regs[i],3,";");
		registro_t* registro = (registro_t*) malloc(tamanio_pagina);

		registro->timestamp = strtoull(campos[0],NULL,10);
		registro->key = atoi(campos[1]);


		registro->value = malloc(strlen(campos[2]) + 1);
		memset(registro->value,'\0',strlen(campos[2]) + 1);
		memcpy(registro->value,campos[2],strlen(campos[2]) + 1);

		bool estaRegistro(registro_t* reg){return reg->key == registro->key;}
		registro_t* registro_encontrado = list_find(lista_registros,(void*) estaRegistro);
		if(registro_encontrado == NULL){
			list_add(lista_registros,registro);

		}else{
			if(registro->timestamp >= registro_encontrado->timestamp){

				list_remove_by_condition(lista_registros,(void*) estaRegistro);
				list_add(lista_registros,registro);
			}else{
				free(registro);
			}
		}


		i++;
	}

	//printf("tam registrossss: %i\n",list_size(lista_registros));


}

void cargar_en_memoria_temporales_a_compactar(char* nom_tabla, t_list* lista_registros){

	char* path_tabla = path_directorio_tabla(nom_tabla);
	struct dirent *ep;
	DIR *dp;
	char* ruta = string_new();
	ruta = string_duplicate(path_tabla);



	dp = opendir (path_tabla);

	if (dp != NULL){
			 while ((ep = readdir (dp))!= NULL){

				 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
					 //Descarto el . y los ..

					 char **nombre_archivo = string_split(ep->d_name, ".");
					 string_to_upper(nombre_archivo[0]);
					 //cargo los .bin(Menos el Metadata.bin) y los .tmpc
					 if((strcmp(nombre_archivo[1],FORMATO_TMP_C) == 0  || strcmp(nombre_archivo[1],FORMATO_BIN_SIN_PUNTO) == 0) & (strcmp(nombre_archivo[0],"METADATA") != 0)){

						 //printf("%s\n",ep->d_name);
						 char* path = path_file(nom_tabla,ep->d_name);
						 char* archivo = subir_archivo_to_memoria(path);
						 agregarlos_to_lista(lista_registros,archivo);
						 free(archivo);

						 //printf("tam registros: %i\n",list_size(lista_registros));
						 free(path);

					 }
				 }
			 }
	}

	free(ruta);
	(void) closedir (dp);
	free(path_tabla);



}

void separar_en_particiones(t_list* lista_registros , t_list* lista_particiones[], int cant_particiones){

	int j;
	for(j=0;j<list_size(lista_registros);j++){

		registro_t* reg = list_get(lista_registros,j);
		int particion = reg->key % cant_particiones;
		//printf("particion: %i\n",particion);

//		printf("reg->key : %i\n",reg->key);
//		printf("reg->value: %s\n",reg->value);
//		printf("pagina->timestamp AMTES: %llu\n",reg->timestamp);
		list_add((t_list*)lista_particiones[particion],reg);



	}

	int k;
	for(k=0;k<list_size(lista_registros);k++){

		registro_t* reg1 = list_remove(lista_registros,k);
		//free(reg1); NO HAY QUE LIBERAR EL NODO YA QUE EL PUNTERO LO AGREGAMOS A LA LISTA lista_particiones

	}

	list_destroy(lista_registros);


}

char* serializar_registro(registro_t* pagina,int tamanio){


	char* buffer = malloc(tamanio + 1);
	memset(buffer,'\0',tamanio + 1);
	int desplazamiento = 0;
	memcpy(buffer + desplazamiento,ull_to_string(pagina->timestamp),string_length(ull_to_string(pagina->timestamp)));
	desplazamiento += string_length(ull_to_string(pagina->timestamp));
	memcpy(buffer + desplazamiento,";",CHAR);
	desplazamiento += CHAR;
	memcpy(buffer + desplazamiento,string_itoa(pagina->key),string_length(string_itoa(pagina->key)));
	desplazamiento += string_length(string_itoa(pagina->key));
	memcpy(buffer + desplazamiento,";",CHAR);
	desplazamiento += CHAR;
	memcpy(buffer + desplazamiento,pagina->value,string_length(pagina->value));
	desplazamiento += string_length(pagina->value);
	memcpy(buffer + desplazamiento,"\n",CHAR);
	desplazamiento += CHAR;
	memcpy(buffer + desplazamiento,"\0",CHAR);
	return buffer;

}

int tamanio_registro(registro_t* pagina){
	//printf("pagina->timestamp : %ull\n",pagina->timestamp);
	//printf("ull_to_string(pagina->timestamp): %s\n",ull_to_string(pagina->timestamp));
//	printf("string_length(ull_to_string(pagina->timestamp)): %i\n",string_length(ull_to_string(pagina->timestamp)));
//	printf("string_length(string_itoa(pagina->key)) : %i\n",string_length(string_itoa(pagina->key)));
//	printf("string_length(pagina->value) : %i\n",string_length(pagina->value));
	//printf("pagina->timestamp: %llu\n",pagina->timestamp);
	char* aux = ull_to_string(pagina->timestamp);

	int tam_tmsp = strlen(aux);
	int tam_key = strlen(string_itoa(pagina->key));
	int tam_value = string_length(pagina->value);

	int tamanio = tam_tmsp + tam_key + tam_value + ( 3 * CHAR);
	free(aux);
	return tamanio;
}


int grabarBloque(char* data, int bloque)
{
	FILE *block;

	char* dirBloque = (char*) malloc(1 + strlen(path_bloques()) + strlen(string_itoa(bloque))+ strlen(".bin"));
	strcpy(dirBloque,path_bloques());
	strcat(dirBloque,string_itoa(bloque));
	strcat(dirBloque,".bin");

	block = fopen (dirBloque, "w");
	//printf("dirBloque: %s\n",dirBloque);
	if(block != NULL)
	{
		fprintf(block,"%s",data);
		bitarray_set_bit(bitarray, bloque);
		fclose (block);
	}else
	{
		printf ("\nNo se pudo grabar el bloque %i", bloque);
	}


	int acum=0;
	int i;
	for(i=0; i<string_length(data); i++) // se fija cuantas veces aparece el \n para no contarlo entre los bytes que dumpeo.
	{
		 if(data[i]=="\n")
		 {
	        acum++;

		 }
	}


	free(dirBloque);


	return string_length(data) - acum;
}

void grabar_bloques_de_particiones(t_list* lista_particiones[],int cant_particiones,t_list* lista_bloques[], int tamanio_particiones[]){

	char* ptr = NULL;
	char* ptr_aux = NULL;
	int tamanio_total = 0;
	int desplazamiento = 0;
	int i;
	for(i=0;i<cant_particiones;i++){
		//t_list* lista_particion = (t_list*) lista_particiones[i];
		int j;
		//printf("list_size(lista_particiones[i]) ES: %i\n", list_size(lista_particiones[i]));
		for(j=0;j<list_size(lista_particiones[i]);j++){

			registro_t* reg = list_get(lista_particiones[i],j);
//			printf("reg->key : %i\n",reg->key);
//			printf("reg->value: %s\n",reg->value);
//			printf("pagina->timestamp AMTES: %llu\n",reg->timestamp);

			int tamanio = tamanio_registro(reg);
			//printf("tamanio: %i\n",tamanio);
			char* buffer = serializar_registro(reg,tamanio);
			free(reg);
			//printf("buffer: %s\n",buffer);

			tamanio_total += tamanio;
			//printf("tamanio_total: %i\n",tamanio_total);
			ptr_aux = realloc(ptr,tamanio_total );
			if(ptr_aux == NULL){
				printf("No hay memoria\n");
			}else{
				ptr = ptr_aux;
			}



			//printf("tam buffer: %i\n", string_length(buffer));
			memcpy(ptr + desplazamiento,buffer,tamanio );
			free(buffer);

			desplazamiento += tamanio;
//			int h;
//			for(h=0;h<tamanio_total;h++){
//				printf("ptr %i: %c\n", h, ptr[h]);
//			}


		}

		tamanio_particiones[i] = tamanio_total + 1;
		//tamanio_total += 1;
		ptr_aux = realloc(ptr,tamanio_total + 1 );
		ptr = ptr_aux;
		ptr[tamanio_total] = '\0';
		//printf("ptr finalll: %s\n",ptr);

		char* ptr_aux = NULL;
		int tam_bloque = config_MetaData.tamanio_bloques;
		//printf("tam_bloque: %i\n",tam_bloque);
		int tamanio_a_grabar = tamanio_total ;
		int desplasamiento = 0;

		while(tamanio_a_grabar > 0){
			int bloque_libre = proximobloqueLibre();
			//printf("bloque_libre : %i\n",bloque_libre);

			if(tamanio_a_grabar >= tam_bloque){
				ptr_aux = malloc(tam_bloque + 1);
				memcpy(ptr_aux,ptr + desplasamiento,tam_bloque);
				memcpy(ptr_aux + tam_bloque,"\0",CHAR);
				int cant_grabada = grabarBloque(ptr_aux,bloque_libre);
				tamanio_a_grabar -= tam_bloque;
				desplasamiento += tam_bloque;
			}else{
				ptr_aux = malloc(tamanio_a_grabar + 1);
				memcpy(ptr_aux,ptr + desplasamiento,tamanio_a_grabar);
				memcpy(ptr_aux + tamanio_a_grabar,"\0",CHAR);
				int cant_grabada = grabarBloque(ptr_aux,bloque_libre);
				tamanio_a_grabar -= tam_bloque;
				desplasamiento += tamanio_a_grabar;
			}

			free(ptr_aux);
			ptr_aux =  NULL;

			int* nro_particion = malloc(INT);
			*nro_particion = bloque_libre;

			list_add(lista_bloques[i],nro_particion);


		}
		//int cant_grabada = grabarBloque(ptr,777);
		//printf("cant_grabada: %i\n",cant_grabada);

	}


}

void liberar_bloques_utilizados(char* path){

	bin_tmp_t* file = (bin_tmp_t*) leer_bin_tmp_tmpc(path);
	//printf("file->size: %i\n",file->size);
	//printf("file->bloques: %s\n",file->bloques);
	quitar_corchetes(file);

	char** bloques = string_n_split(file->bloques, CANT_MAX_BLOQUES_POR_PARTICION, ",");

	int j = 0;
	int tamanio_total = 0;

	while(bloques[j] != NULL){

		bitarray_clean_bit(bitarray,j);

		j++;
	}

	free(file);
}

char* crear_lista_de_bloques(int nro_particion,int* lista_bloques[]){

	char* ptr = NULL;
	char* ptr_aux = NULL;
	int tam_total = 0;
	int desplazamiento = 0;
	int m;
	for(m=0;m<list_size(lista_bloques[nro_particion]);m++){
		int* b = list_get(lista_bloques[nro_particion],m);
		char* bc = string_itoa(*b);
		int tam = strlen(bc);
		tam_total += tam;
		ptr_aux = realloc(ptr,tam_total);
		ptr = ptr_aux;
		memcpy(ptr + desplazamiento,bc,tam);
		desplazamiento += tam;

		tam_total += CHAR;
		ptr_aux = realloc(ptr,tam_total);
		ptr = ptr_aux;
		memcpy(ptr + desplazamiento,",",CHAR);
		desplazamiento += CHAR;
	}

	tam_total += CHAR;
	ptr_aux = realloc(ptr,tam_total);
	ptr = ptr_aux;
	memcpy(ptr + desplazamiento,"\0",CHAR);

	//printf("tam_total: %i\n",tam_total);
	//printf("desplazamiento: %i\n",desplazamiento);
	ptr[desplazamiento -1] = '\0';
	//printf("bloques: %s\n",ptr);
	return ptr;
}


char* crear_contenido_bin(int nro_particion,int* lista_bloques[], int tamanios_particiones[]){

	char* bloques = crear_lista_de_bloques(nro_particion,lista_bloques);
	int tam_part = tamanios_particiones[nro_particion];
	char* str_tam_part = string_itoa(tam_part);
	int tam_total = strlen("SIZE=") + strlen(str_tam_part) + strlen("\n") + strlen("BLOCKS=[") + strlen(bloques) + strlen("]\0") + CHAR;

	char* buffer = malloc(tam_total);
	int desplazamiento = 0;
	memcpy(buffer + desplazamiento, "SIZE=",strlen("SIZE="));
	desplazamiento += strlen("SIZE=");
	memcpy(buffer + desplazamiento, str_tam_part,strlen(str_tam_part));
	desplazamiento += strlen(str_tam_part);
	memcpy(buffer + desplazamiento, "\n",CHAR);
	desplazamiento += CHAR;
	memcpy(buffer + desplazamiento, "BLOCKS=[",strlen("BLOCKS=["));
	desplazamiento += strlen("BLOCKS=[");
	memcpy(buffer + desplazamiento, bloques,strlen(bloques));
	desplazamiento += strlen(bloques);
	memcpy(buffer + desplazamiento, "]",CHAR);
	desplazamiento += CHAR;
	memcpy(buffer + desplazamiento, "\0",CHAR);
	//printf("buffer bin: %s\n",buffer);
	free(bloques);

	return buffer;



}

void crear_particiones_nuevas(int cant_particiones,int* lista_bloques[],int tamanios_particiones[], char* nom_tabla){

	char* path_tabla = path_directorio_tabla(nom_tabla);
	struct dirent *ep;
	DIR *dp;
	char* ruta = string_new();
	ruta = string_duplicate(path_tabla);



	dp = opendir (path_tabla);

	if (dp != NULL){
			 while ((ep = readdir (dp))!= NULL){

				 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
					 //Descarto el . y los ..

					 char **nombre_archivo = string_split(ep->d_name, ".");
					 int nro_part = atoi(nombre_archivo[0]);
					 string_to_upper(nombre_archivo[0]);
					 //cargo los .bin(Menos el Metadata.bin) y los .tmpc
					 if((strcmp(nombre_archivo[1],FORMATO_BIN_SIN_PUNTO) == 0 || strcmp(nombre_archivo[1],FORMATO_TMP_C) == 0) & (strcmp(nombre_archivo[0],"METADATA") != 0)){

						 char* path = path_file(nom_tabla,ep->d_name);
						 if(strcmp(nombre_archivo[1],FORMATO_TMP_C) == 0){
							 //printf("%s\n",ep->d_name);
							 char* path = path_file(nom_tabla,ep->d_name);
							 liberar_bloques_utilizados(path);

							 remove(path);
							 free(path);

						 }else{
							 //printf("%s\n",ep->d_name);
							 char* path = path_file(nom_tabla,ep->d_name);

							 char* contenido = crear_contenido_bin(nro_part,lista_bloques,tamanios_particiones);
							 liberar_bloques_utilizados(path);
							 remove(path);
							 FILE *block;
							 block = fopen (path, "wb");
							 if(block != NULL)
							 {
								fprintf(block,"%s",contenido);

								fclose (block);
							 }else
							 {
								printf ("\nNo se pudo grabar el contenido  %i", contenido);
							 }

							 free(contenido);
							 free(path);
						 }


					 }
				 }
			 }
	}

	free(ruta);
	(void) closedir (dp);
	free(path_tabla);


}

void ejecutar_compactar(char* nom_tabla){

	t_list* lista_registros = list_create();


	renombrar_temporales(nom_tabla);
	cargar_en_memoria_temporales_a_compactar(nom_tabla,lista_registros);
	t_config_MetaData_tabla* metadata =  leer_metaData_tabla(nom_tabla);
	int cant_particiones = metadata->partitions;

	free(metadata);
	t_list* lista_particiones[cant_particiones];
	t_list* lista_bloques[cant_particiones];
	int tamanios_particiones[cant_particiones];

	int j;
	for(j=0;j<cant_particiones;j++){
		lista_particiones[j] = list_create();
		lista_bloques[j] = list_create();
	}
	//printf("lista_registros ANTES: %i\n", list_size(lista_registros));
	separar_en_particiones(lista_registros,lista_particiones,cant_particiones);
	//printf("list_size(lista_particiones[0]): %i\n", list_size(lista_particiones[0]));
	grabar_bloques_de_particiones(lista_particiones,cant_particiones,lista_bloques,tamanios_particiones);

	int d;
	for(d=0;d<list_size(lista_bloques[0]);d++){
		int* blo = list_get(lista_bloques[0],d);
		//printf("blo: %i\n",*blo);
	}

	crear_particiones_nuevas(cant_particiones,lista_bloques,tamanios_particiones,nom_tabla);


}

void compactar(void* nombre_tabla){

	char* nom_tabla = (char*) nombre_tabla;
	//printf("nom_tabla: %s\n", nom_tabla);
	t_config_MetaData_tabla* metadata =  leer_metaData_tabla(nom_tabla);
	int time = metadata->compaction_time;
	free(metadata);

	while(TRUE)
	{
		usleep(time * 1000);
		//ejecutar_compactar(nom_tabla);
	}


}


int ejecutar_comando_dump(){
	ejecutar_dump();
	return OK;
}
int ejecutar_comando_compactar(char* nombre_tabla){

	ejecutar_compactar(nombre_tabla);
	return OK;
}

int ejecutar_comando_create(char* nombre_tabla,char* consistency, int cant_partitions, int compaction_time )
{


	int index_segmento;
	index_segmento = existe_tabla(nombre_tabla);
	if(index_segmento < 0)
	{//La tabla no existe y la tenemos que agregar.

		int result = crear_tabla_fs(nombre_tabla,consistency, cant_partitions, compaction_time);
		if( result == 1)
		 {
			pthread_t hilo_compactar;
			//pthread_create(&unCliente->hilo, NULL, &cliente, (void*) sockCliente);
			pthread_create(&hilo_compactar, NULL, (void*) compactar, (void*) nombre_tabla);
			//printf ("La tabla %s fue creada exitosamente.\n", nombre_tabla);
		 }else
		 {
			 switch(result)
			 {
			 	 case -1: printf("La creación de la tabla falló al momento de crear el directorio de la tabla.\n");
			 	 	 	  break;
			 	 case -2: printf("La creación de la tabla falló al momento de crear el archivo metadata de la tabla.\n");
			 			  break;
			 	 case -3: printf("La creación de la tabla falló al momento de crear las particiones de la tabla.\n");
			 			  break;
			 	 case -4: printf("La creación de la tabla falló al momento de agregar la tabla en la memtable.\n");
			 			  break;
			 	 case -5: printf("La tabla %s ya existe.\n", nombre_tabla);
			 	 	 	  break;
			 	 default: break;
			 }
		 }
	}else{///La tabla SI existe
		printf("La tabla que intenta crear ya existe\n");
		log_error(logger," ERROR. La tabla que intenta crear ya existe.");
	}

	return EXIT_SUCCESS;
}



int get_registro_memtable(int index_tabla, int key ){

	int index=-1;
	unsigned long long timestamp_max = 0;
	tabla_t* nodo_tabla = list_get(lista_tablas,index_tabla);
	int i = 0;

	for(; i < list_size(nodo_tabla->registros); i++) {
		registro_t* reg = (registro_t*) list_get(nodo_tabla->registros,i);
		if(reg->key == key){
			if(reg->timestamp > timestamp_max){
				timestamp_max = reg->timestamp;
				index = i;

			}
		}
	}


	return index;

}


//char* get_registro_memtable(int index_tabla, int key ){
//
//	char* valor_max = malloc(tamanio_value);
//	unsigned long timestamp_max = 0;
//	tabla_t* nodo_tabla = list_get(lista_tablas,index_tabla);
//	int i = 0;
//
//	for(; i < list_size(nodo_tabla->registros); i++) {
//		registro_t* reg = (registro_t*) list_get(nodo_tabla->registros,i);
//		if(reg->key == key){
//			if(reg->timestamp > timestamp_max){
//				timestamp_max = reg->timestamp;
//				memset(valor_max,'\0',tamanio_value);
//				memcpy(valor_max,reg->value,tamanio_value);
//
//			}
//		}
//	}
//
//
//	return valor_max;
//
//}

int ejecutar_comando_registros(char* nombre_tabla)
{
	int i = 0;
	int index_tabla;
	index_tabla = existe_tabla(nombre_tabla);

	if(index_tabla >= 0)
	{
		tabla_t* tabla = list_get(lista_tablas,index_tabla);
		for(; i < list_size(tabla->registros); i++) {
			registro_t* registro = (registro_t*) list_get(tabla->registros,i);
			printf("Timestamp: %llu  Key: %i  Valor: %s  \n", registro->timestamp, registro->key, registro->value);

		}
	}


	return EXIT_SUCCESS;
}

int ejecutar_comando_tablas()
{
	int i = 0;
	for(; i < list_size(lista_tablas); i++) {
		tabla_t* tabla = (tabla_t*) list_get(lista_tablas,i);
		printf("Nombre tabla: %s  cantidad registros: %i  consistency: %s  cant partitions: %i  compaction time: %i  \n", tabla->nombre_tabla, tabla->cant_reg, tabla->consistency, tabla->cant_partitions, tabla->compaction_time);


	}

	return EXIT_SUCCESS;
}

char* ull_to_string(unsigned long long valor){
	char buffer[100] = {'\0'};
	int tamanio = sprintf(buffer,"%llu",valor);
	char* valor_char = malloc(tamanio + 1);
	memset(valor_char,'\0',tamanio+1);
	strcpy(valor_char,buffer);

	//printf("valor_char: %s\n",valor_char);
	return valor_char;
}

char* get_valor(char* nombre_tabla, int key){

	char* valor_final = malloc(tamanio_value + CHAR + UNSIGNED_LONG_LONG + CHAR);
	memset(valor_final,'\0',tamanio_value);

	char* valor_bin_tmp = malloc(tamanio_value);
	memset(valor_bin_tmp,'\0',tamanio_value);
	unsigned long long  timestamp_bin_tmp = 0;
	timestamp_bin_tmp = buscar_en_temporales_y_binarios(nombre_tabla,key,valor_bin_tmp);

	printf("timestamp_bin_tmp: %llu\n",timestamp_bin_tmp);

	char* valor_memtable = malloc(tamanio_value);
	memset(valor_memtable,'\0',tamanio_value);
	unsigned long long tsmp_memtable = 0;
	tsmp_memtable = buscar_en_memtable(nombre_tabla,key,valor_memtable);

	printf("tsmp_memtable: %llu\n",tsmp_memtable);

	if(tsmp_memtable > timestamp_bin_tmp ){

		printf("El timstamp mas grande es: %llu\n",tsmp_memtable);
		printf("La key es: %i\n",key);
		printf("El value es: %s\n",valor_memtable);

		free(valor_bin_tmp);


		char* time_aux = ull_to_string(tsmp_memtable);


		string_append(&valor_final,valor_memtable);
		string_append(&valor_final," ");
		string_append(&valor_final,time_aux);

		free(valor_memtable);
		free(time_aux);


		return valor_final;


	}

	if(timestamp_bin_tmp > tsmp_memtable){

		printf("El timstamp mas grande es: %llu\n",timestamp_bin_tmp);
		printf("La key es: %i\n",key);
		printf("El value es: %s\n",valor_bin_tmp);

		free(valor_memtable);


		char* time_aux = ull_to_string(timestamp_bin_tmp);


		string_append(&valor_final,valor_bin_tmp);
		string_append(&valor_final," ");
		string_append(&valor_final,time_aux);

		free(valor_bin_tmp);
		free(time_aux);

		return valor_final;


	}



	if((timestamp_bin_tmp == tsmp_memtable) & tsmp_memtable == 0){
		printf("son iguales");
		return strcpy(valor_final,NO_EXISTE_LA_KEY);
	}
}
//
//char* get_valor(char* nombre_tabla, int key){
//
//	char* valor_memtable = malloc(tamanio_value);
//	memset(valor_memtable,'\0',tamanio_value);
//	char* valor_temp = malloc(tamanio_value);
//	memset(valor_temp,'\0',tamanio_value);
//	char* valor_bin = malloc(tamanio_value);
//	memset(valor_bin,'\0',tamanio_value);
//
//	char* valor_final = malloc(tamanio_value + CHAR + UNSIGNED_LONG_LONG + CHAR);
//	memset(valor_final,'\0',tamanio_value);
//
//	t_config_MetaData_tabla* metadata = leer_metaData_tabla(nombre_tabla);
//	//int cant_particiones = get_cant_particiones(nombre_tabla);
//	int cant_particiones = metadata->partitions;
//	free(metadata);
//
//	//printf("%i\n",cant_particiones);
//	int index_tabla;
//	unsigned long long tsmp_memtable = 0;
//	index_tabla = existe_tabla(nombre_tabla);
//	if(index_tabla >= 0){//Si existe la tabla entonces busco en la memtable
//		//printf("/*************************** Empiezo busqueda en memtable*********************\n");
//		tsmp_memtable = buscar_en_memtable(nombre_tabla,key,valor_memtable);
////		printf("/*************************** Fin busqueda en memtable: Resultados:*********************\n");
////		printf("El timstamp es: %llu\n",tsmp_memtable);
////		printf("La key es: %i\n",key);
////		printf("El value es: %s\n",valor_memtable);
//
//	}
//
//	//printf("/*************************** Empiezo busqueda en temporales*********************\n");
//	unsigned long long tsmp_temporal = buscar_en_temporales(nombre_tabla,key,valor_temp);
////	printf("/*************************** Fin busqueda en temporales: Resultados:*********************\n");
////	printf("El timstamp  es: %llu\n",tsmp_temporal);
////	printf("La key es: %i\n",key);
////	printf("El value es: %s\n",valor_temp);
//
//	unsigned long long tsmp_bin = 0;
//	int index_tabla_disco = existe_tabla_en_disco(nombre_tabla);
//	if(index_tabla_disco == -1){//Si existe la tabla entonces busco en la memtable
//		//Si no tengo la cantidad de particiones entonces no puedo buscar en que bin esta la key.
//		//printf("/*************************** Empiezo busqueda en bin*********************\n");
//		tsmp_bin = buscar_en_bin(nombre_tabla,key,cant_particiones,valor_bin);
////		printf("/*************************** Fin busqueda en bin: Resultados:*********************\n");
////		printf("El timstamp  es: %llu\n",tsmp_bin);
////		printf("La key es: %i\n",key);
////		printf("El value es: %s\n",valor_bin);
//	}
//
//	//printf("/*************************** RESULTADO FINAL*********************\n");
//
//	if((tsmp_memtable > tsmp_temporal) & (tsmp_memtable > tsmp_bin) ){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_memtable);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_memtable);
//
//		free(valor_temp);
//		free(valor_bin);
//
//		char* time_aux = ull_to_string(tsmp_memtable);
//
//
//		string_append(&valor_final,valor_memtable);
//		string_append(&valor_final," ");
//		string_append(&valor_final,time_aux);
//
//		free(valor_memtable);
//		free(time_aux);
//
//
//		return valor_final;
//
//
//	}
//
//	if((tsmp_temporal > tsmp_memtable) & (tsmp_temporal > tsmp_bin)){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_temporal);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_temp);
//
//		free(valor_memtable);
//		free(valor_bin);
//
//		char* time_aux = ull_to_string(tsmp_temporal);
//
//
//		string_append(&valor_final,valor_temp);
//		string_append(&valor_final," ");
//		string_append(&valor_final,time_aux);
//
//		free(valor_temp);
//		free(time_aux);
//
//		return valor_final;
//
//
//	}
//
//	if((tsmp_bin > tsmp_memtable) & (tsmp_bin > tsmp_temporal) ){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_bin);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_bin);
//
//		free(valor_temp);
//		free(valor_memtable);
//
//		char* time_aux = ull_to_string(tsmp_bin);
//
//		string_append(&valor_final,valor_bin);
//		string_append(&valor_final," ");
//		string_append(&valor_final,time_aux);
//
//		free(valor_bin);
//		free(time_aux);
//
//		return valor_final;
//
//	}
//
//	free(valor_temp);
//	free(valor_memtable);
//	free(valor_bin);
//
//	return valor_final;
//
//}

void ejecutar_comando_journal(char* mensaje, int socket_mem){


	char** parametros = string_n_split(mensaje,4,"|");


	int result = ejecutar_comando_insert( parametros[0], strtol(parametros[1],NULL,10),  parametros[2],  strtoull(parametros[3],NULL,10));
	int bytes_enviados;

	if(result ==  1){
		bytes_enviados = aplicar_protocolo_enviar(socket_mem,JOURNAL,SUCCESSFUL);

	}else{
		bytes_enviados = aplicar_protocolo_enviar(socket_mem,JOURNAL,NO_EXISTE_LA_TABLA);
	}


}

void ejecutar_comando_select_memoria(char* query, int socket_mem){

	char** campos = string_n_split(query,3," ");
	char* nombre_tabla = campos[1];
	int key = atoi(campos[2]);


	char* valor = get_valor(nombre_tabla, key);

	printf("valor:%s\n",valor);
	if(valor != NULL){
		aplicar_protocolo_enviar(socket_mem,SELECT,valor);
	}else{
		aplicar_protocolo_enviar(socket_mem,SELECT, NO_EXISTE_LA_KEY );
	}

	free(valor);

}
//
//void ejecutar_comando_select_memoria(char* query, int socket_mem){
//
//	char** campos = string_n_split(query,3," ");
//	char* nombre_tabla = campos[1];
//	int key = atoi(campos[2]);
//
//
//
//	char* valor = get_valor(nombre_tabla, key);
//
//
//	if(valor != NULL){
//		char* valor_a_enviar = malloc(strlen(valor) + 1 );
//
//		memset(valor_a_enviar,'\0',strlen(valor)+ 1);
//		memcpy(valor_a_enviar,valor,strlen(valor));
//
//		free(valor);
//		aplicar_protocolo_enviar(socket_mem,SELECT,valor_a_enviar);
//
//		free(valor_a_enviar);
//	}else{
//		aplicar_protocolo_enviar(socket_mem,SELECT, NO_EXISTE_LA_KEY );
//	}
//
//
//
//}

int  ejecutar_comando_select(char* nombre_tabla, int key){



	char* valor = get_valor(nombre_tabla, key);

	if(valor != NULL){
		printf("valor: %s\n",valor);
	}else{
		printf("No existe la key \n");
	}

	free(valor);

	return OK;
}

//
//int ejecutar_comando_select(char* nombre_tabla, int key){
//
//	char* valor_memtable = malloc(tamanio_value);
//	char* valor_temp = malloc(tamanio_value);
//	memset(valor_temp,'\0',tamanio_value);
//	char* valor_bin = malloc(tamanio_value);
//
//	t_config_MetaData_tabla* metadata = leer_metaData_tabla(nombre_tabla);
//	//int cant_particiones = get_cant_particiones(nombre_tabla);
//	int cant_particiones = metadata->partitions;
//	free(metadata);
//
//	printf("%i\n",cant_particiones);
//	int index_tabla;
//	unsigned long tsmp_memtable = 0;
//	index_tabla = existe_tabla(nombre_tabla);
//	if(index_tabla >= 0){//Si existe la tabla entonces busco en la memtable
//		printf("/*************************** Empiezo busqueda en memtable*********************\n");
//		tsmp_memtable = buscar_en_memtable(nombre_tabla,key,valor_memtable);
//		printf("/*************************** Fin busqueda en memtable: Resultados:*********************\n");
//		printf("El timstamp es: %llu\n",tsmp_memtable);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_memtable);
//
//	}
//
//	printf("/*************************** Empiezo busqueda en temporales*********************\n");
//	unsigned long tsmp_temporal = buscar_en_temporales(nombre_tabla,key,valor_temp);
//	printf("/*************************** Fin busqueda en temporales: Resultados:*********************\n");
//	printf("El timstamp  es: %llu\n",tsmp_temporal);
//	printf("La key es: %i\n",key);
//	printf("El value es: %s\n",valor_temp);
//
//	unsigned long tsmp_bin = 0;
//	int index_tabla_disco = existe_tabla_en_disco(nombre_tabla);
//	if(index_tabla_disco == -1){//Si existe la tabla entonces busco en la memtable
//		//Si no tengo la cantidad de particiones entonces no puedo buscar en que bin esta la key.
//		printf("/*************************** Empiezo busqueda en bin*********************\n");
//		tsmp_bin = buscar_en_bin(nombre_tabla,key,cant_particiones,valor_bin);
//		printf("/*************************** Fin busqueda en bin: Resultados:*********************\n");
//		printf("El timstamp  es: %llu\n",tsmp_bin);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_bin);
//	}
//
//	printf("/*************************** RESULTADO FINAL*********************\n");
//
//	if((tsmp_memtable > tsmp_temporal) & (tsmp_memtable > tsmp_bin) ){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_memtable);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_memtable);
//
//	}
//
//	if((tsmp_temporal > tsmp_memtable) & (tsmp_temporal > tsmp_bin)){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_temporal);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_temp);
//
//	}
//
//	if((tsmp_bin > tsmp_memtable) & (tsmp_bin > tsmp_temporal) ){
//
//		printf("El timstamp mas grande es: %llu\n",tsmp_bin);
//		printf("La key es: %i\n",key);
//		printf("El value es: %s\n",valor_bin);
//
//	}
//	free(valor_memtable);
//	free(valor_temp);
//	free(valor_bin);
//	return EXIT_SUCCESS;
//
//}

char* quitar_comillas(char* valor){
	char* valor_sin_comillas;
	int lengt_valor = strlen(valor);
	//printf("lengt_valor: %i\n",lengt_valor);
	valor_sin_comillas = string_substring(valor,1,(lengt_valor-2));
	//printf("tamaño valor_sin_comillas: %i\n",strlen(valor_sin_comillas));
	//printf("valor_sin_comillas: %s\n",valor_sin_comillas);
	return valor_sin_comillas;
}

void agregar_registro(int index_tabla, int key, char* valor, unsigned long long timestamp){

	 tabla_t* nodo_tabla = list_get(lista_tablas,index_tabla);
	//Creo el nodo para el registro////
	registro_t* registro = (registro_t*) malloc(tamanio_pagina);

	//char* valor_sin_comillas = quitar_comillas(valor);

	registro->timestamp = timestamp;
	registro->key = key;
	registro->value = malloc(tamanio_value);
	memset(registro->value,'\0',tamanio_value);
	//memcpy(registro->value,valor,strlen(valor) + 1);
	memcpy(registro->value,valor,strlen(valor) + 1);

	list_add(nodo_tabla->registros,registro);
	nodo_tabla->cant_reg += 1;
}

int agregar_tabla(char* nombre_tabla,char* consistency, int cant_partitions, int compaction_time ){


	/// Creación del nodo tabla
	tabla_t* nodo_tabla = (tabla_t*) malloc(sizeof(tabla_t));
	if(nodo_tabla == NULL)
	{
		printf("No hay memoria para un registro en la tabla de tablas\n");
		return -1;
	}

	strcpy(nodo_tabla->nombre_tabla,nombre_tabla);
	nodo_tabla->cant_reg = 0;

	strcpy(nodo_tabla->consistency,consistency);
	nodo_tabla->cant_partitions = cant_partitions;
	nodo_tabla->compaction_time = compaction_time;
	nodo_tabla->registros = list_create();


	list_add(lista_tablas,nodo_tabla);
	return 1;

}

int ejecutar_comando(char* comando) {
	int ret = 0;
	char* comando_listo = comando_preparado(comando);
	char** parametros = string_n_split(comando_listo, 6, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_AYUDA)) {
		printf("Se ejecuto comando ayuda\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_SELECT)) {
		//printf("Se ejecuto comando select\n");
		ret = ejecutar_comando_select(parametros[1],strtol(parametros[2],NULL,10));
		usleep(configFile->retardo * 1000); // Retardo por operacion.
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_INSERT)) {

		char* timestamp_aux = NULL;
		char** parametros_insert = string_n_split(comando_listo, 3,"\"");
		//printf("parametros_insert[0]: %s\n",parametros_insert[0]);
		//printf("parametros_insert[1]: %s\n",parametros_insert[1]);
		//printf("parametros_insert[2]: %s\n",parametros_insert[2]);
		if(parametros_insert[2] != NULL){
			//printf("parametros_insert[22]: %s\n",parametros_insert[2]);
			char** parms = string_n_split(parametros_insert[2],2," ");
			//printf("parms[0]: %s\n",parms[0]);
			timestamp_aux = parms[0];
		}

		//printf("parametros_insert: %s\n",parametros_insert[1]);
		//if(parametros[4] == NULL){
		if(timestamp_aux == NULL){
			ret = ejecutar_comando_insert( parametros[1], strtol(parametros[2],NULL,10),  parametros_insert[1],  NULL);
		}else{
			//ret = ejecutar_comando_insert( parametros[1], strtol(parametros[2],NULL,10),  parametros_insert[1],  strtoull(parametros[4],NULL,10));
			ret = ejecutar_comando_insert( parametros[1], strtol(parametros[2],NULL,10),  parametros_insert[1],  strtoull(timestamp_aux,NULL,10));
		}

		usleep(configFile->retardo * 1000); // Retardo por operacion.
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_CREATE)) {
		if (parametros[1] == NULL || parametros[2] == NULL || parametros[3] == NULL || parametros[4] == NULL) {
			printf("Faltan ingresar parametros\n");
			return ERROR;
		}

		ret = ejecutar_comando_create(parametros[1],parametros[2],strtol(parametros[3],NULL,10),strtol(parametros[4],NULL,10));
		usleep(configFile->retardo * 1000); // Retardo por operacion.
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DESCRIBE)) {
		if(parametros[1]!= NULL )
		{
			ejecutar_comando_describe(parametros[1]);
		}else
		{
			ejecutar_comando_describe("");
		}

		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DROP)) {
		if(parametros[1]!= NULL )
		{
			ret = ejecutar_comando_drop(parametros[1]);
		}

		usleep(configFile->retardo * 1000); // Retardo por operacion.
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_JOURNAL)) {
		printf("Comando no encontrado. journal\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_ADD)) {
		printf("Comando no encontrado. add\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_RUN)) {
		printf("Comando no encontrado. run\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_METRICS)) {
		printf("Comando no encontrado. metrics\n");
		return ret;

	} else if (string_equals_ignore_case(parametros[0], COMANDO_TABLAS)) {
		ret = ejecutar_comando_tablas();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_REGISTROS)) {
		if (parametros[1] == NULL ) {
					printf("Faltan ingresar parametros\n");
					return ERROR;
		}
		ret = ejecutar_comando_registros(parametros[1]);
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_COMPACTAR)) {
		if (parametros[1] == NULL ) {
					printf("Faltan ingresar parametros\n");
					return ERROR;
		}
		ret = ejecutar_comando_compactar(parametros[1]);
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_DUMP)) {
		printf("ejecutar_comando_dump\n");
		ret = ejecutar_comando_dump();
		return ret;

	}else {
		log_error(logger,"Comando incorrecto ingrese ayuda para una lista de comandos validos o salir para cerrar el sistema");
		return 1;
	}



	return 0;
}

int get_cant_particiones(char* nombre_tabla){

	int cant_particiones = 0;
	int index_tabla;
	index_tabla = existe_tabla(nombre_tabla);
	if(index_tabla >= 0){
		tabla_t* nodo_tabla = list_get(lista_tablas,index_tabla);
		cant_particiones = nodo_tabla->cant_partitions;
	}

	return cant_particiones;

}


//short get_tamanio_archivo(char * path_archivo){
//
//
//	//La función que llama a esta funcios se DEBE encargar de hacer un FREE a archivo
//	FILE *archivo = fopen(path_archivo, "r");
//	if (archivo == NULL)
//	{
//		printf("Error de apertura en script :%s\n\n",archivo);
//		fclose(archivo);
//		return 0;
//	}
//
//	//free(archivo_bloque);
//
//	short i = 0;
//	while (!feof(archivo))
//	{
//		fgetc(archivo);
//		i++;
//	}
//	fclose(archivo);//fclose hace el free de archivo
//
//	return i;
//}


char * get_content_archivo(char * path_bloque, int size)
{
	char *cadenaRetorno;
	cadenaRetorno = malloc(size + 1);
	strcpy(cadenaRetorno, "");


	FILE * archivoScript = fopen(path_bloque, "r");
	if (archivoScript == NULL)
	{
		printf("Error de apertura en script:%s\n\n",path_bloque);
		return "\0";
	}
	else
	{	/*Recorro el archivo bloque y lo voy anexando linea por linea*/
			do
			{
				char cadena[MAX_LINEA];
				fgets(cadena,MAX_LINEA,archivoScript);
				//printf("cadena: %s\n",cadena);
				strcat(cadenaRetorno,cadena);
				//printf("cadenaRetorno: %s\n",cadenaRetorno);
				//printf("strlen(cadenaRetorno): %i\n",strlen(cadenaRetorno));
			}
			//while ( !feof(archivoScript) );
			while ( !feof(archivoScript) && (strlen(cadenaRetorno)<size) );

			fclose(archivoScript);
	}


	return cadenaRetorno;
}

short get_tamanio_archivo(char * path_bloque){

	//char* nombreScript=getNombreScript(path_bloque);
	//FILE *archivoScript = fopen(nombreScript, "r");
	FILE *archivo_bloque = fopen(path_bloque, "r");
	if (archivo_bloque == NULL)
	{
		printf("Error de apertura en script :%s\n\n",archivo_bloque);
		fclose(archivo_bloque);
		return 0;
	}

	//free(archivo_bloque);

	short i = 0;
	while (!feof(archivo_bloque))
	{
		fgetc(archivo_bloque);
		i++;
	}
	fclose(archivo_bloque);
	return i;
}


//char * get_content_archivo(char * path_archivo, int size)
//{
//	char *cadenaRetorno;
//	cadenaRetorno = malloc(size + 1);
//	strcpy(cadenaRetorno, "");
//
//
//	FILE * archivo = fopen(path_archivo, "r");
//	if (archivo == NULL)
//	{
//		printf("Error de apertura en archivo:%s\n\n",path_archivo);
//		return "\0";
//	}
//	else
//	{	/*Recorro el archivo  y lo voy anexando linea por linea*/
//			do
//			{
//				char cadena[MAX_LINEA];
//				fgets(cadena,MAX_LINEA,archivo);
//				strcat(cadenaRetorno,cadena);
//			}
//			while ( !feof(archivo) && (strlen(cadenaRetorno)<size) );
//
//			fclose(archivo);
//	}
//
//
//	return cadenaRetorno;
//}
//
//int buscar_valor_en_bin()
//{
//	bin_tmp_t* file_bin = malloc(sizeof(bin_tmp_t));
//	t_config *config;
//
//	char* nombre_tabla = "tabla1";
//
//	int key = 3;
//	char* key_s = string_itoa(key);
//
//	int cant_particiones = 2;
//
//	int particion_int = key % cant_particiones;
//
//	char* nombre_particion = string_itoa(particion_int);
//
//	char* path_tabla = string_duplicate(configFile->puntoMontaje);
//	string_append(&path_tabla,TABLES);
//	string_append(&path_tabla,nombre_tabla);
//	string_append(&path_tabla,BARRA);
//	string_append(&path_tabla,nombre_particion);
//	string_append(&path_tabla,FORMATO_BIN);
//
//
//
//
//	config = config_create(path_tabla);
//
//	//config = config_create("../mnt/LISSANDRA_FS/Tables/tabla1/1.bin");
//
//	if (config != NULL) {
//
//		if (config_has_property(config, "SIZE")) {
//			file_bin->size = config_get_int_value(config,
//							"SIZE");
//		} else {
//				log_error(logger,
//				"El archivo bin no contiene el SIZE");
//
//		}
//
//		if (config_has_property(config, "BLOCKS")) {
//			file_bin->bloques = strdup(
//									config_get_string_value(config, "BLOCKS"));
//
//		} else {
//				log_error(logger,
//				"El archivo bin  no contiene  el parametro block");
//		}
//	}
//
//
//	/**********Le quitamos los corchetes a la lista de bloques*////////////
//	char* blo = file_bin->bloques;
//	int tam = strlen(file_bin->bloques);
//
//	blo[tam - 1] = '\0';
//
//	int p=0;
//	for(;p<(tam -1);p++){
//		blo[p] = blo[p+1];
//	}
//	//printf("el valor de blo es %s\n",blo);
//	char** bloques = string_n_split(blo, CANT_MAX_BLOQUES_POR_PARTICION, ",");
//	/**************************************************************/
//
//
//
//
//	int y=0;
//	while(bloques[y] != NULL){
//
//		//printf("El indice %i tiene valor %s\n", y,bloques[y]);
//
//		char* path_bloque = string_duplicate(configFile->puntoMontaje);
//		string_append(&path_bloque, PATH_BLOQUES);
//
//		string_append(&path_bloque, bloques[y]);
//		string_append(&path_bloque, FORMATO_BIN);
//
//		//printf("El path del bloque es %s\n", path_bloque);
//
//		char *archivo_bloque;
//		/*OBTENGO EL TAMAÑO DEL ARCHIVO DE SCRIPT */
//		int tamanioArchivo = 0;
//		if (!(tamanioArchivo = get_tamanio_archivo(path_bloque) -1))
//				return ERROR;
//
//
//
//		/*PASO EL CONTENIDO DEL ARCHIVO  A UN STRING*/
//		archivo_bloque = get_content_archivo(path_bloque, tamanioArchivo);
//
//		if (!strcmp(archivo_bloque, "\0"))
//			return ERROR;
//
//		archivo_bloque[tamanioArchivo-1]='\0';
//
//
//
//		char** registro = string_n_split(archivo_bloque, MAX_LINEA, "\n");
//
//		int h=0;
//		while(registro[h] != NULL){
//			//printf("El valor del registro es %s\n",registro[h]);
//			char** campos = string_n_split(registro[h], 3, ";");
//
//			//printf("La key %s tiene valor %s\n",campos[0],campos[2]);
//
//
//			if(!strcmp(campos[0],key_s)){
//				printf("El valor de la key %i es: %s\n",key,campos[2]);
//			}
//
//
//			h += 1;
//		}
//
//		y += 1;
//
//
//		free(path_bloque);
//		free(archivo_bloque);
//
//	}
//
//
//
//	free(key_s);
//	free(nombre_particion);
//	free(path_tabla);
//
//	return EXIT_SUCCESS;
//}

unsigned long long buscar_en_archivo(char* path_archivo,int key,char* valor){


	unsigned long long timestamp_max = 0;
	bin_tmp_t* file_bin = malloc(sizeof(bin_tmp_t));
	t_config *config;
	char* key_buscada = string_itoa(key);


	//Obtengo el contendido del archivo tmp/tmpc/bin
	config = config_create(path_archivo);

	if (config != NULL) {

		if (config_has_property(config, "SIZE")) {
			file_bin->size = config_get_int_value(config,
							"SIZE");
		} else {
				log_error(logger,
				"El archivo bin no contiene el SIZE");

		}

		if (config_has_property(config, "BLOCKS")) {
			file_bin->bloques = strdup(
									config_get_string_value(config, "BLOCKS"));

		} else {
				log_error(logger,
				"El archivo bin  no contiene  el parametro block");
		}
	}

	//printf("El valor de Size es %i\n", file_bin->size);
	//printf("El valor de Blocks es %s\n", file_bin->bloques);

	/**********Le quitamos los corchetes a la lista de bloques*////////////
	char* blo = file_bin->bloques;
	int tam = strlen(file_bin->bloques);

	blo[tam - 1] = '\0';

	int p=0;
	for(;p<(tam -1);p++){
		blo[p] = blo[p+1];
	}
	//printf("el valor de blo es %s\n",blo);
	char** bloques = string_n_split(blo, CANT_MAX_BLOQUES_POR_PARTICION, ",");
	/**************************************************************/




	int y=0;
	while(bloques[y] != NULL){
		//Por cada nombre de bloque voy a buscar y abrir el archivo bloque

		//printf("El indice %i tiene valor %s\n", y,bloques[y]);

		char* path_bloque = string_duplicate(configFile->puntoMontaje);
		string_append(&path_bloque, PATH_BLOQUES);

		string_append(&path_bloque, bloques[y]);
		string_append(&path_bloque, FORMATO_BIN);

		//printf("El path del bloque es %s\n", path_bloque);

		char *archivo_bloque;
		/*OBTENGO EL TAMAÑO DEL ARCHIVO DE SCRIPT */
		int tamanioArchivo = 0;
		if (!(tamanioArchivo = get_tamanio_archivo(path_bloque) -1))
				return ERROR;

		//printf("El tamanio archivo es %i\n",tamanioArchivo);

		/*PASO EL CONTENIDO DEL ARCHIVO  A UN STRING*/
		archivo_bloque = get_content_archivo(path_bloque, tamanioArchivo);

		if (!strcmp(archivo_bloque, "\0"))
			return ERROR;

		archivo_bloque[tamanioArchivo]='\0';

		//printf("%s\n", archivo_bloque);

		char** registro = string_n_split(archivo_bloque, MAX_LINEA, "\n");

		int h=0;
		while(registro[h] != NULL){
			//printf("El valor del registro es %s\n",registro[h]);
			char** campos = string_n_split(registro[h], 3, ";");

			//printf("La key %s tiene valor %s\n",campos[0],campos[2]);


			if(!strcmp(campos[0],key_buscada)){
				printf("El valor de la key %i es: %s\n",key,campos[2]);
				//Si la key es la misma comparo el timestamp
				unsigned long long timestamp_encontrado = (unsigned long long) strtoull(campos[1],NULL,10);
				if(timestamp_encontrado > timestamp_max){
					timestamp_max = timestamp_encontrado;
					printf("El timestamp max es  %llu\n",timestamp_encontrado);
					strcpy(valor,campos[2]);

				}
			}


			h += 1;
		}

		y += 1;


		free(path_bloque);
		free(archivo_bloque);


	}



	free(key_buscada);


	return timestamp_max;

}
unsigned long long buscar_en_memtable(char* nombre_tabla, int key, char* valor){
	int index_tabla;
	int index_registro;


	index_tabla = existe_tabla(nombre_tabla);
	registro_t* nodo_reg;
	if(index_tabla >= 0){//La tabla SI existe

		printf("index_tabla: %i\n",index_tabla);
		tabla_t* nodo_tabla = (tabla_t*) list_get(lista_tablas,index_tabla);
		index_registro = get_registro_memtable(index_tabla,  key);

		if(index_registro != -1){
			nodo_reg = (registro_t*) list_get(nodo_tabla->registros,index_registro);
			strcpy(valor,nodo_reg->value);
			return nodo_reg->timestamp;
		}




	}

	return 0;


}


//
//unsigned long long buscar_en_temporales(char* nombre_tabla, int key, char* valor){
//		//Busca en TODOS los temporales
//        //char **parametros = string_split("../mnt/LISSANDRA_FS/Tables/tabla1/", " ");
//
//		unsigned long long timestamps_max = 0;
//        char* path_tabla = malloc(100);//Liberar este malloc
//        strcpy(path_tabla,PATH_TABLAS);
//        string_append(&path_tabla,nombre_tabla);
//        string_append(&path_tabla,BARRA);
//        DIR *dp;
//        struct dirent *ep;
//        dp = opendir (path_tabla);
//        if (dp != NULL){
//                 while ((ep = readdir (dp))!= NULL){
//                     //Por cada archivo temporal busco la key
//                	 //y me quedo con el value que tenga el time stamp
//                	 //mas grande
//                	 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
//                		 //Descarto el . y los ..
//                		 printf("%s\n",ep->d_name);
//                		 char **nombre_archivo = string_split(ep->d_name, ".");
//
//                		 if(!strcmp(nombre_archivo[1],FORMATO_TMP) || !strcmp(nombre_archivo[1],FORMATO_TMP_C) ){
//
//                			char* path_tabla_temporal = malloc(100);//Liberar este malloc
//							strcpy(path_tabla_temporal,PATH_TABLAS);
//							string_append(&path_tabla_temporal,nombre_tabla);
//							string_append(&path_tabla_temporal,BARRA);
//							string_append(&path_tabla_temporal,ep->d_name);
//							//OBTENGO EL TAMAÑO DEL ARCHIVO DEL ARCHIVO
//							//printf("el path del temp es %s\n",path_tabla_temporal);
//							char*  valor_aux = malloc(tamanio_value);
//							unsigned long long timestamps_actual = buscar_en_archivo(path_tabla_temporal,key,valor_aux);
//							if(timestamps_actual > timestamps_max )
//							{
//								timestamps_max = timestamps_actual;
//								strcpy(valor,valor_aux);
//							}
//							free(valor_aux);
//
//
//							free(path_tabla_temporal);
//
//                		 }
//
//
//                	 }
//
//
//
//
//
//
//                 }
//        (void) closedir (dp);
//        }
//        else{
//                          perror ("No puede abrir el directorio");
//        }
//
//
//        free(path_tabla);
//
//        return timestamps_max;
//}


unsigned long long buscar_en_registros(t_list* lista_registros,int key_buscada, char* valor){

	unsigned long long timestamp_max = 0;
	unsigned long long timestamp_aux = 0;
	int key_aux;
	int i;
	for(i=0;i<list_size(lista_registros);i++){
		registro_t* reg = list_get(lista_registros,i);
		timestamp_aux = reg->timestamp;
		key_aux = reg->key;
		if(key_aux == key_buscada){

			if(timestamp_aux > timestamp_max){
				timestamp_max = timestamp_aux;
				strcpy(valor,reg->value);

			}
		}

	}

	return timestamp_max;

}


unsigned long long buscar_en_temporales_y_binarios(char* nombre_tabla, int key, char* valor){
		//Busca en TODOS los temporales
        //char **parametros = string_split("../mnt/LISSANDRA_FS/Tables/tabla1/", " ");

		t_list* lista_registros = list_create();
		char*  valor_aux = NULL;
		unsigned long long timestamps_aux = 0;
		unsigned long long timestamps_max = 0;
        char* path_tabla = malloc(100);//Liberar este malloc
        strcpy(path_tabla,PATH_TABLAS);
        string_append(&path_tabla,nombre_tabla);
        string_append(&path_tabla,BARRA);
        DIR *dp;
        struct dirent *ep;
        dp = opendir (path_tabla);
        if (dp != NULL){
                 while ((ep = readdir (dp))!= NULL){
                     //Por cada archivo  busco la key
                	 //y me quedo con el value que tenga el timestamp
                	 //mas grande

                	 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
                		 //Descarto el . y los ..
                		 printf("%s\n",ep->d_name);
                		 char **nombre_archivo = string_split(ep->d_name, ".");
                		 string_to_upper(nombre_archivo[0]);
                		 if((strcmp(nombre_archivo[1],FORMATO_TMP) == 0) ||( strcmp(nombre_archivo[1],FORMATO_TMP_C) == 0) || ((strcmp(nombre_archivo[1],FORMATO_BIN_SIN_PUNTO) == 0) & (strcmp(nombre_archivo[0],"METADATA") != 0)) ){


							 printf("%s\n",ep->d_name);
							 char* path = path_file(nombre_tabla,ep->d_name);
							 char* archivo = subir_archivo_to_memoria(path);
							 free(path);
							 agregarlos_to_lista(lista_registros,archivo);
							 free(archivo);

							 //printf("tam registros: %i\n",list_size(lista_registros));


							 valor_aux = malloc(tamanio_value);
							 timestamps_aux = buscar_en_registros(lista_registros,key,valor_aux);
							 if(timestamps_aux > timestamps_max )
							 {
								timestamps_max = timestamps_aux;
								strcpy(valor,valor_aux);
							 }

							 free(valor_aux);
							 valor_aux = NULL;

							 list_clean(lista_registros);


                		 }


                	 }






                 }
        (void) closedir (dp);
        }
        else{
                          perror ("No puede abrir el directorio");
        }


        free(path_tabla);

        return timestamps_max;
}
unsigned long long buscar_en_bin(char* nombre_tabla, int key, int cant_particiones, char* valor){


	bin_tmp_t* file_bin = malloc(sizeof(bin_tmp_t));
	t_config *config;
	unsigned long long timptstamp_max = 0;

	char* key_buscada = string_itoa(key);

	int particion_int = key % cant_particiones;

	char* nombre_particion = string_itoa(particion_int);

	char* path_tabla = string_duplicate(configFile->puntoMontaje);
	string_append(&path_tabla,TABLES);
	string_append(&path_tabla,nombre_tabla);
	string_append(&path_tabla,BARRA);
	string_append(&path_tabla,nombre_particion);
	string_append(&path_tabla,FORMATO_BIN);


	//printf("El valor de la particion es %s\n",path_tabla);

	config = config_create(path_tabla);



	if (config != NULL) {

		if (config_has_property(config, "SIZE")) {
			if( config_get_string_value(config,"SIZE") != NULL){
				file_bin->size = config_get_int_value(config,
											"SIZE");
			}


		} else {
				log_error(logger,
				"El archivo bin no contiene el SIZE");

		}

		if (config_has_property(config, "BLOCKS")) {
			file_bin->bloques = strdup(
									config_get_string_value(config, "BLOCKS"));

		} else {
				log_error(logger,
				"El archivo bin  no contiene  el parametro block");
		}
	}



	/**********Le quitamos los corchetes a la lista de bloques*////////////
	char* blo = file_bin->bloques;
	int tam = strlen(file_bin->bloques);

	blo[tam - 1] = '\0';

	int p=0;
	for(;p<(tam -1);p++){
		blo[p] = blo[p+1];
	}
	//printf("el valor de blo es %s\n",blo);
	char** bloques = string_n_split(blo, CANT_MAX_BLOQUES_POR_PARTICION, ",");
	/**************************************************************/




	int y=0;
	while(bloques[y] != NULL){

		//printf("El indice %i tiene valor %s\n", y,bloques[y]);

		char* path_bloque = string_duplicate(configFile->puntoMontaje);
		string_append(&path_bloque, PATH_BLOQUES);

		string_append(&path_bloque, bloques[y]);
		string_append(&path_bloque, FORMATO_BIN);

		//printf("El path del bloque es %s\n", path_bloque);

		char *archivo_bloque;
		/*OBTENGO EL TAMAÑO DEL ARCHIVO DE SCRIPT */
		int tamanioArchivo = 0;
		if (!(tamanioArchivo = get_tamanio_archivo(path_bloque) -1))
				return ERROR;

		//printf("El tamanio archivo es %i\n",tamanioArchivo);

		/*PASO EL CONTENIDO DEL ARCHIVO  A UN STRING*/
		archivo_bloque = get_content_archivo(path_bloque, tamanioArchivo);

		if (!strcmp(archivo_bloque, "\0"))
			return ERROR;

		archivo_bloque[tamanioArchivo-1]='\0';

		//printf("%s\n", archivo_bloque);

		char** registro = string_n_split(archivo_bloque, MAX_LINEA, "\n");

		int h=0;
		while(registro[h] != NULL){
			//printf("El valor del registro es %s\n",registro[h]);
			char** campos = string_n_split(registro[h], 3, ";");

			//printf("La key %s tiene valor %s\n",campos[0],campos[2]);


			if(!strcmp(campos[0],key_buscada)){
//				printf("El valor de la key %i es: %s\n",key,campos[2]);
				unsigned long long timptstamp_aux = (unsigned long long) strtoull(campos[1],NULL,10);
				if(timptstamp_aux > timptstamp_max){
					timptstamp_max = timptstamp_aux;
					strcpy(valor,campos[2]);
				}
			}


			h += 1;
		}

		y += 1;


		free(path_bloque);
		free(archivo_bloque);

	}



	free(key_buscada);
	free(nombre_particion);
	free(path_tabla);

	return timptstamp_max;

}


char* crear_lista_de_bloques_for_dump(t_list* lista_bloques){

	char* ptr = NULL;
	char* ptr_aux = NULL;
	int tam_total = 0;
	int desplazamiento = 0;
	int m;
	for(m=0;m<list_size(lista_bloques);m++){
		int* b = list_get(lista_bloques,m);
		char* bc = string_itoa(*b);
		int tam = strlen(bc);
		tam_total += tam;
		ptr_aux = realloc(ptr,tam_total);
		ptr = ptr_aux;
		memcpy(ptr + desplazamiento,bc,tam);
		desplazamiento += tam;

		tam_total += CHAR;
		ptr_aux = realloc(ptr,tam_total);
		ptr = ptr_aux;
		memcpy(ptr + desplazamiento,",",CHAR);
		desplazamiento += CHAR;
	}

	tam_total += CHAR;
	ptr_aux = realloc(ptr,tam_total);
	ptr = ptr_aux;
	memcpy(ptr + desplazamiento,"\0",CHAR);

	printf("tam_total: %i\n",tam_total);
	printf("desplazamiento: %i\n",desplazamiento);
	ptr[desplazamiento -1] = '\0';
	printf("bloques: %s\n",ptr);
	return ptr;
}

int dump_crear_tmp(char *nombre_tabla, int size, t_list* lista_bloques)
{

	char* bloquesUsados = crear_lista_de_bloques_for_dump(lista_bloques);
	int nroTmp = 1;

	DIR *dp;
	struct dirent *ep;
	char* tmpMontaje = path_punto_montaje_tables();

	char* dir_tabla= (char*) malloc(string_length(nombre_tabla)  + string_length(tmpMontaje) + 1);
	strcpy(dir_tabla,tmpMontaje);
	strcat(dir_tabla, nombre_tabla);

	free(tmpMontaje);


	dp = opendir (dir_tabla); //reviso si no existe otro archivo .tmp que aún no ha sido compactado para no repetir nombre
	if (dp != NULL)
	{
		while ((ep = readdir (dp))!= NULL)
		{
			if(string_ends_with(ep->d_name,".tmp"))
			{
				nroTmp++; //si encuentra un tmp, incrementa el numerador para crear .tmp nuevo
			}

		}
		closedir (dp);

	}
	FILE *fTmp;
	int mallocTmp = 1+ string_length(dir_tabla) + string_length(string_itoa(nroTmp)) + string_length("/.tmp");
	char* dirTmp = (char*) malloc(mallocTmp);
	strcpy(dirTmp,dir_tabla);
	strcat(dirTmp,"/");
	strcat(dirTmp,string_itoa(nroTmp));
	strcat(dirTmp,".tmp");


	fTmp = fopen(dirTmp,"w");
	if(fTmp != NULL )
	{
		fprintf(fTmp,"SIZE=%i\nBLOCKS=[%s]",size,bloquesUsados);
	}

	fclose(fTmp);
	free(dirTmp);
	free(dir_tabla);


	int h;
	for(h=0;h<list_size(lista_bloques);h++){
		int* b  = list_get(lista_bloques,h);
		free(b);
	}
	free(bloquesUsados);
	bloquesUsados=NULL;
	return 1;
}

void limpiar_memtable(){
	int i;
	for(i=0;i<list_size(lista_tablas);i++){
		tabla_t* tabla = list_get(lista_tablas,i);
		list_clean(tabla->registros);
	}

	list_clean(lista_tablas);
}

void ejecutar_dump(void)
{


	int i;
	tabla_t* tabla = NULL;

	//pthread_mutex_lock(&mx_tablas);
	for (i=0; i < list_size(lista_tablas); i++) //recorre una a una las tablas
	{
		tabla = (tabla_t*) list_get(lista_tablas, i); //carga la tabla en memoria

		char* ptr = NULL;
		char* ptr_aux = NULL;
		int tamanio_total = 0;
		int desplazamiento = 0;
		t_list* lista_bloques = list_create();
		int j;
		for(j=0;j<list_size(tabla->registros);j++){
			//Recorro uno a uno los registros serializandolos en un buffer
			registro_t* reg = list_get(tabla->registros,j);

//			printf("reg->key : %i\n",reg->key);
//			printf("reg->value: %s\n",reg->value);
//			printf("pagina->timestamp AMTES: %llu\n",reg->timestamp);

			int tamanio = tamanio_registro(reg);
			//printf("tamanio: %i\n",tamanio);
			char* buffer = serializar_registro(reg,tamanio);
			free(reg);
			//printf("buffer: %s\n",buffer);

			tamanio_total += tamanio;
			//printf("tamanio_total: %i\n",tamanio_total);
			ptr_aux = realloc(ptr,tamanio_total );
			if(ptr_aux == NULL){
				printf("No hay memoria\n");
			}else{
				ptr = ptr_aux;
			}



			//printf("tam buffer: %i\n", string_length(buffer));
			memcpy(ptr + desplazamiento,buffer,tamanio );
			free(buffer);

			desplazamiento += tamanio;
			int h;
			for(h=0;h<tamanio_total;h++){
				//printf("ptr %i: %c\n", h, ptr[h]);
			}
		}


		ptr_aux = realloc(ptr,tamanio_total + 1 );
		ptr = ptr_aux;
		ptr[tamanio_total] = '\0';
		//printf("ptr finalll: %s\n",ptr);

		ptr_aux = NULL;
		int tam_bloque = config_MetaData.tamanio_bloques;
		//printf("tam_bloque: %i\n",tam_bloque);
		int tamanio_a_grabar = tamanio_total ;
		int desplasamiento = 0;

		//Una vez que tengo el buffer con la info de una tabla los guardo en los
		//bloques, guardando los nros de bloques utilizados
		while(tamanio_a_grabar > 0){
			int bloque_libre = proximobloqueLibre();
			printf("bloque_libre : %i\n",bloque_libre);

			if(tamanio_a_grabar >= tam_bloque){//Si lo que voy a grabar ocupa mas de un bloque
				ptr_aux = malloc(tam_bloque + 1);
				memcpy(ptr_aux,ptr + desplasamiento,tam_bloque);
				memcpy(ptr_aux + tam_bloque,"\0",CHAR);
				grabarBloque(ptr_aux,bloque_libre);
				tamanio_a_grabar -= tam_bloque;
				desplasamiento += tam_bloque;
			}else{//Si lo que voy a grabar ocupa menos de un bloque
				ptr_aux = malloc(tamanio_a_grabar + 1);
				memcpy(ptr_aux,ptr + desplasamiento,tamanio_a_grabar);
				memcpy(ptr_aux + tamanio_a_grabar,"\0",CHAR);
				grabarBloque(ptr_aux,bloque_libre);
				tamanio_a_grabar -= tam_bloque;
				desplasamiento += tamanio_a_grabar;
			}

			free(ptr_aux);
			ptr_aux =  NULL;

			int* nro_particion = malloc(INT);
			*nro_particion = bloque_libre;

			list_add(lista_bloques,nro_particion);


		}

		dump_crear_tmp(tabla->nombre_tabla,tamanio_total,lista_bloques);


	}

	limpiar_memtable();

}



/*********** Funciones de Bitmap ********************************************************/
int cantidadDeBloquesLibres (){
	size_t	cantidadDebits= bitarray_get_max_bit (bitarray);
	int libre =0;
	int i;
	for (i=0;i<cantidadDebits;i++){
		if (bitarray_test_bit(bitarray,i)==0){
			libre++;
		}
	}
	return libre;
}
int proximobloqueLibre (){
	size_t	cantidadDebits= bitarray_get_max_bit (bitarray);
	int i;
	int libre=0;
	for (i=0;i<cantidadDebits;i++){
		if(bitarray_test_bit(bitarray,i)==0){
			libre=i;
			break;
		}
	}
	return libre;
}



t_config_MetaData_tabla* leer_metaData_tabla(char* nombre_tabla){

	char* path_metadata_tabla = path_metadata(nombre_tabla);

	//printf("path_metadata_tabla: %s\n",path_metadata_tabla);
	t_config* archivo_MetaData;
	t_config_MetaData_tabla* config_MetaData_tabla = malloc(sizeof(t_config_MetaData_tabla));

	archivo_MetaData=config_create(path_metadata_tabla);
	if(archivo_MetaData == NULL){
		printf("No se pudo abrir el archivo metadata.bin\n");
	}


	config_MetaData_tabla->nombre_tabla = (char*) malloc(strlen(nombre_tabla) + 1);
	strcpy(config_MetaData_tabla->nombre_tabla,nombre_tabla);

	config_MetaData_tabla->consistency = (char*)malloc(4);
	strcpy(config_MetaData_tabla->consistency,string_duplicate(config_get_string_value(archivo_MetaData,"CONSISTENCY")));

	config_MetaData_tabla->partitions=config_get_int_value(archivo_MetaData,"PARTITIONS");
	config_MetaData_tabla->compaction_time=config_get_int_value(archivo_MetaData,"COMPACTION_TIME");

	free(path_metadata_tabla);
	config_destroy(archivo_MetaData);

	return config_MetaData_tabla;
}


bin_tmp_t* leer_bin_tmp_tmpc(char* path){

	t_config* config_file;
	bin_tmp_t* file = malloc(sizeof(bin_tmp_t));

	config_file = config_create(path);

	if (config_file != NULL) {

		if (config_has_property(config_file, "SIZE")) {
			if( config_get_string_value(config_file,"SIZE") != NULL){
				file->size = config_get_int_value(config_file,"SIZE");
			}

		} else {
				log_error(logger,
				"El archivo bin no contiene el SIZE");

		}

		if (config_has_property(config_file, "BLOCKS")) {
			file->bloques = strdup(
									config_get_string_value(config_file, "BLOCKS"));

		} else {
				log_error(logger,
				"El archivo bin  no contiene  el parametro block");
		}
	}

	//free(path);
	config_destroy(config_file);

	return file;

}

void quitar_corchetes(bin_tmp_t* file){

	/**********Le quitamos los corchetes a la lista de bloques*////////////



	int tam = strlen(file->bloques);
	file->bloques[tam - 1] = '\0';
	int p=0;
	for(;p<(tam -1);p++){
		file->bloques[p] = file->bloques[p+1];
	}

	//printf("el file->bloques  es %s\n",file->bloques);
}

//void borrar_archivos_bloques(char* nombre_tabla, char* nombre_file){
void borrar_archivos_bloques(char* path){
	bin_tmp_t* file = leer_bin_tmp_tmpc(path);
	printf("bloques: %s\n",file->bloques);
	quitar_corchetes(file);
	printf("file->bloques: %s\n", file->bloques);
	char** bloques = string_n_split(file->bloques, CANT_MAX_BLOQUES_POR_PARTICION, ",");


	int j = 0;
	while(bloques[j] != NULL){

		char* path = path_bloque(bloques[j]);
		printf("path bloque: %s\n",path);
		remove(path);
		free(path);
		j++;
	}



	free(file);

}

void drop_tabla_memtable(char* nombre_tabla){

	int index_segmento = existe_tabla(nombre_tabla);
	if(index_segmento >= 0)
	{
		tabla_t* aux = (tabla_t*) list_remove(lista_tablas, index_segmento);
		int j;
		for(j=0;j<list_size(aux->registros);j++){
			registro_t* reg = list_remove(aux->registros,j);
			free(reg);
		}

		free(aux);
	}
}

void ejecutar_comando_drop_memoria(char* query, int socket){

	char** parametros = string_n_split(query,2," ");

	ejecutar_comando_drop(parametros[1]);

	aplicar_protocolo_enviar(socket,DROP,SUCCESSFUL);

}
int ejecutar_comando_drop(char* nombre_tabla)
{
	printf("nombre_tabla: %s\n",nombre_tabla);
	char* path = path_directorio_tabla(nombre_tabla);
	printf("path: %s\n",path);

	drop_tabla_memtable(nombre_tabla);

	DIR *dp;
	struct dirent *ep;
	dp = opendir (path);
	if (dp != NULL){
			 while ((ep = readdir (dp))!= NULL){
				 //Por cada archivo temporal busco la key
				 //y me quedo con el value que tenga el time stamp
				 //mas grande
				 if(!(!strcmp(ep->d_name,PUNTO) || !strcmp(ep->d_name,DOS_PUNTOS))  ){
					 //Descarto el . y los ..
					 //char* nom_file = string_duplicate(ep->d_name);
					 printf("tam nom: %i\n",strlen(ep->d_name));
					 printf("%s\n",ep->d_name);

					 char* path_archivo = path_file(nombre_tabla,ep->d_name);
					 printf("path_archivo: %s\n",path_archivo);

					 string_to_upper(ep->d_name);

					 if(strcmp(ep->d_name,"METADATA.BIN") != 0){//Si es Metadata.bin no entra aca
						 borrar_archivos_bloques(path_archivo );
					 }

					 remove(path_archivo);
					 free(path_archivo);


				 }
			 }
	}


	//Borro el directorio tabla
	if(remove(path) == 0 ){
		free(path);
		return OK;
	}else{
		free(path);
		return ERROR;
	}




}

void ejecutar_comando_describe(char* nombre_tabla)
{
	int i;
	t_list * lista_resultados = get_resultado_describe(nombre_tabla);

	for(i=0; i < list_size(lista_resultados) ;i++){

			t_config_MetaData_tabla* metadata = list_get(lista_resultados,i);
			printf("\n\nTabla = %s\nConsistency= %s\nPartitions=%i\nCompaction Time=%i\n", metadata->nombre_tabla, metadata->consistency,metadata->partitions,metadata->compaction_time);

	}

	int j;
	for(j=0; j < list_size(lista_resultados) ;j++){

		t_config_MetaData_tabla* metadata = list_get(lista_resultados,j);
		free(metadata->nombre_tabla);
		free(metadata->consistency);
		free(metadata);
	}



}


void ejecutar_comando_describe_para_memoria(char* query, int sockMemoria){

	char **consulta;
	consulta = string_n_split(query, 2, " ");

	t_list * resultado;
	if(consulta[1] == NULL){
		//printf("la tabla es NULL\n");
		resultado = get_resultado_describe("");
	}else{
		resultado = get_resultado_describe(consulta[1]);
	}


	char* buffer = armar_respuesta_describe(resultado);
	int bytes_enviados = aplicar_protocolo_enviar(sockMemoria,DESCRIBE,buffer);


	free(buffer);

}

//
//void ejecutar_comando_describe_para_memoria(char* query, int sockMemoria){
//
//	char **consulta;
//	consulta = string_n_split(query, 2, "|");
//
//	t_list * resultado;
//	if(consulta[1] == NULL){
//		//printf("la tabla es NULL\n");
//		resultado = get_resultado_describe("");
//	}else{
//		resultado = get_resultado_describe(consulta[1]);
//	}
//
//
//
//	free(query);
//
//
//
//	char* buffer = armar_respuesta_describe(resultado);
//	int bytes_enviados = aplicar_protocolo_enviar(sockMemoria,DESCRIBE,buffer);
//
//
//	free(buffer);
//
//}


void liberar_memoria(t_list* list_punteros){
	int j;
	for(j=0;j<list_size(list_punteros);j++){
		char* registro = (char*)list_get(list_punteros,j);
		free(registro); registro =NULL;

	}
}

char* serializar_metadatas(t_list* list_punteros, int bytes_total){


	int j;

//	int cant_tablas = list_size(list_punteros);
//
//	char* cantidad_tablas = string_itoa(cant_tablas);
//	int tam_cantidad_tablas = strlen(cantidad_tablas);
//
//	bytes_total += tam_cantidad_tablas + 1;
	char* buffer = (char*) malloc(bytes_total + 1);
	memset(buffer,'\0',bytes_total + 1);
//
//	memcpy(buffer,cantidad_tablas,tam_cantidad_tablas);
//	memcpy(buffer + tam_cantidad_tablas,"*",1);
//	int desplazamiento = tam_cantidad_tablas + 1;

	int desplazamiento = 0;
	for(j=0;j<list_size(list_punteros);j++){
		char* registro = (char*)list_get(list_punteros,j);
		int tam_registro = string_length(registro);
		memcpy(buffer + desplazamiento,registro,tam_registro);
		desplazamiento += tam_registro;

	}

	return buffer;


}
//char* serializar_metadatas(t_list* list_punteros, int bytes_total){
//
//
//	int j;
//
//	int cant_tablas = list_size(list_punteros);
//
//	char* cantidad_tablas = string_itoa(cant_tablas);
//	int tam_cantidad_tablas = strlen(cantidad_tablas);
//
//	bytes_total += tam_cantidad_tablas + 1;
//	char* buffer = (char*) malloc(bytes_total);
//
//	memcpy(buffer,cantidad_tablas,tam_cantidad_tablas);
//	memcpy(buffer + tam_cantidad_tablas,"*",1);
//	int desplazamiento = tam_cantidad_tablas + 1;
//	for(j=0;j<list_size(list_punteros);j++){
//		char* registro = (char*)list_get(list_punteros,j);
//		int tam_registro = string_length(registro);
//		memcpy(buffer + desplazamiento,registro,tam_registro);
//		desplazamiento += tam_registro;
//
//	}
//
//	return buffer;
//
//
//}
int calcular_tamanio_respuesta_describe(t_list* list_punteros){

	int j;
	int bytes_total=0;
	for(j=0;j<list_size(list_punteros);j++){
		char* registro = (char*)list_get(list_punteros,j);

		bytes_total =  bytes_total + strlen(registro);

	}

	return bytes_total;
}

char* armar_respuesta_describe(t_list* lista_resultados){

	t_list* list_punteros = list_create();

	int i;
	char* ptr_aux = NULL;
	for(i=0; i < list_size(lista_resultados) ;i++){

		t_config_MetaData_tabla* metadata = list_get(lista_resultados,i);
		int desplazamiento = 0;

		int tam_nom_tabla = string_length(metadata->nombre_tabla);
		int tam_consistencia = strlen(metadata->consistency);
		int tam_partitions =  string_length(string_itoa(metadata->partitions));
		int tam_compation =  string_length(string_itoa(metadata->compaction_time));
		int tam_bytes = tam_nom_tabla + tam_consistencia + tam_partitions +tam_compation + (3 * CHAR) + CHAR;
//		printf("ttam_nom_tabla: %i\n",tam_nom_tabla);
//		printf("tam_consistencia: %i\n",tam_consistencia);
//		printf("tam_partitions: %i\n",tam_partitions);
//		printf("tam_compation: %i\n",tam_compation);
//		printf("tam_bytes: %i\n",tam_bytes);

		ptr_aux = (char*) malloc(tam_bytes + 1 );
		memset(ptr_aux,'\0',tam_bytes + 1);
		if(ptr_aux == NULL){
			printf("error malloc\n");
		}

		memcpy(ptr_aux,metadata->nombre_tabla,tam_nom_tabla);
		strcpy(ptr_aux,metadata->nombre_tabla);
		desplazamiento += tam_nom_tabla;
		memcpy(ptr_aux + desplazamiento," ",CHAR);
		desplazamiento += CHAR;
		memcpy(ptr_aux + desplazamiento,metadata->consistency,tam_consistencia);
		desplazamiento += tam_consistencia;
		memcpy(ptr_aux + desplazamiento," ",CHAR);
		desplazamiento += CHAR;
		memcpy(ptr_aux + desplazamiento, string_itoa(metadata->partitions),tam_partitions);
		desplazamiento += tam_partitions;
		memcpy(ptr_aux + desplazamiento," ",CHAR);
		desplazamiento += CHAR;
		memcpy(ptr_aux + desplazamiento,string_itoa(metadata->compaction_time),tam_compation);
		desplazamiento += tam_compation;
		memcpy(ptr_aux + desplazamiento,"\n",CHAR);


		list_add(list_punteros,ptr_aux);

	}

	int j;
	for(j=0;j<list_size(list_punteros);j++){
		char* registro = (char*)list_get(list_punteros,j);
	}

	int bytes_total = calcular_tamanio_respuesta_describe(list_punteros);
	char* metadatas = serializar_metadatas(list_punteros,bytes_total);
	liberar_memoria(list_punteros);

	return metadatas;
}

//char* armar_respuesta_describe(t_list* lista_resultados){
//
//	t_list* list_punteros = list_create();
//
//	int i;
//	char* ptr_aux = NULL;
//	for(i=0; i < list_size(lista_resultados) ;i++){
//
//		t_config_MetaData_tabla* metadata = list_get(lista_resultados,i);
//		int desplazamiento = 0;
//
//		int tam_nom_tabla = string_length(metadata->nombre_tabla);
//		int tam_consistencia = strlen(metadata->consistency);
//		int tam_partitions =  string_length(string_itoa(metadata->partitions));
//		int tam_compation =  string_length(string_itoa(metadata->compaction_time));
//		int tam_bytes = tam_nom_tabla + tam_consistencia + tam_partitions +tam_compation + (3 * CHAR) + CHAR;
////		printf("ttam_nom_tabla: %i\n",tam_nom_tabla);
////		printf("tam_consistencia: %i\n",tam_consistencia);
////		printf("tam_partitions: %i\n",tam_partitions);
////		printf("tam_compation: %i\n",tam_compation);
////		printf("tam_bytes: %i\n",tam_bytes);
//
//		ptr_aux = (char*) malloc(tam_bytes);
//		if(ptr_aux == NULL){
//			printf("error malloc\n");
//		}
//
//		memcpy(ptr_aux,metadata->nombre_tabla,tam_nom_tabla);
//		strcpy(ptr_aux,metadata->nombre_tabla);
//		desplazamiento += tam_nom_tabla;
//		memcpy(ptr_aux + desplazamiento,"|",CHAR);
//		desplazamiento += CHAR;
//		memcpy(ptr_aux + desplazamiento,metadata->consistency,tam_consistencia);
//		desplazamiento += tam_consistencia;
//		memcpy(ptr_aux + desplazamiento,"|",CHAR);
//		desplazamiento += CHAR;
//		memcpy(ptr_aux + desplazamiento, string_itoa(metadata->partitions),tam_partitions);
//		desplazamiento += tam_partitions;
//		memcpy(ptr_aux + desplazamiento,"|",CHAR);
//		desplazamiento += CHAR;
//		memcpy(ptr_aux + desplazamiento,string_itoa(metadata->compaction_time),tam_compation);
//		desplazamiento += tam_compation;
//		memcpy(ptr_aux + desplazamiento,"\n",CHAR);
//
//
//		list_add(list_punteros,ptr_aux);
//
//	}
//
//	int j;
//	for(j=0;j<list_size(list_punteros);j++){
//		char* registro = (char*)list_get(list_punteros,j);
//	}
//
//	int bytes_total = calcular_tamanio_respuesta_describe(list_punteros);
//	char* metadatas = serializar_metadatas(list_punteros,bytes_total);
//	liberar_memoria(list_punteros);
//
//	return metadatas;
//}

t_list* get_resultado_describe(char* nombre_tabla){

	t_list* lista_resultados = list_create();
	if(!strcmp(nombre_tabla, ""))
	{


			/*obtengo los directorios*/

				DIR *dp;
				struct dirent *ep;

				char* path_tables = path_punto_montaje_tables();
				dp = opendir (path_tables);
				if (dp != NULL)
				{
					while ((ep = readdir (dp))!= NULL)
					{
						if(strcmp(ep->d_name,".")!= 0 && strcmp(ep->d_name,"..") != 0)
						{
							  if (ep->d_type == DT_DIR)
							  {
								    char* nom_tabla;
								    nom_tabla = (char*) malloc (1 + string_length(ep->d_name));
								    strcpy(nom_tabla,ep->d_name);

//								    printf("nom_tabla es: %s\n",nom_tabla);
								    t_config_MetaData_tabla* metadata  = leer_metaData_tabla(nom_tabla);

								    list_add(lista_resultados,metadata);


							  }
						}

					}
					(void) closedir (dp);
				}
				else
				{
					  perror ("No puede abrir el directorio");


				}

				free(path_tables);


		}else{


			 //printf("nombre_tabla: %s\n",nombre_tabla);
			 t_config_MetaData_tabla* metadata  = leer_metaData_tabla(nombre_tabla);
			 //printf("metadata nombre tabla: %s\n",metadata->nombre_tabla);;

			 list_add(lista_resultados,metadata);



		}


			 return lista_resultados;
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
	case DESCRIBE: case CREATE: case SELECT: case JOURNAL: case DROP:{
		buffer = malloc(tamanio);
		memcpy(buffer, mensaje, tamanio);
		break;
	}

  } // fin switch head
	return buffer;
}

int calcularTamanioMensaje(int head, void* mensaje){

	int tamanio;

	switch(head){
		// CASE 1: El mensaje es un texto (char*)
			case DESCRIBE: case CREATE: case SELECT: case JOURNAL: case DROP:{
				tamanio = strlen((char*) mensaje) + 1;
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




void cargar_tablas_existentes(void)
{

	char *nombre_tabla="";
	ejecutar_comando_describe(nombre_tabla);

}
/**************************************/
char *path_bitmap(){
	char *direccionArchivoBitMap=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen("/Metadata/Bitmap.bin"));
	strcpy(direccionArchivoBitMap,configFile->puntoMontaje);
	string_append(&direccionArchivoBitMap,"Metadata/Bitmap.bin");
	//puts(direccionArchivoBitMap);
	//printf("Direccion de bitmap %s\n",direccionArchivoBitMap);
	return direccionArchivoBitMap;

}
char* path_punto_montaje_tables(){
	char *path=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(MONTAJE_TABLES));
	strcpy(path,configFile->puntoMontaje);
	string_append(&path,MONTAJE_TABLES);

	return path;
}
char *path_metadata(char* nombre_tabla){
	char *path=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(MONTAJE_TABLES) +string_length(nombre_tabla)+ strlen(MONTAJE_METADATA_PUNTO_BIN));
	strcpy(path,configFile->puntoMontaje);
	string_append(&path,MONTAJE_TABLES);
	string_append(&path,nombre_tabla);
	string_append(&path,MONTAJE_METADATA_PUNTO_BIN);


	return path;

}
char* path_directorio_tabla(char* nombre_tabla){
	char *path=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(MONTAJE_TABLES) +string_length(nombre_tabla) + CHAR + CHAR);
	strcpy(path,configFile->puntoMontaje);
	string_append(&path,MONTAJE_TABLES);
	string_append(&path,nombre_tabla);
	//string_append(&path,"\0");


	return path;

}

char* path_file(char* nombre_tabla, char* nombre_archivo){
	char *path=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(MONTAJE_TABLES) +string_length(nombre_tabla)+ CHAR+ strlen(nombre_archivo));
	strcpy(path,configFile->puntoMontaje);
	string_append(&path,MONTAJE_TABLES);
	string_append(&path,nombre_tabla);
	string_append(&path,"/");
	string_append(&path,nombre_archivo);


	return path;

}

char* path_bloque(char* nro_bloque){

	char *path_bloque =(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(PATH_BLOQUES) +string_length(nro_bloque)+  strlen(nro_bloque));
	strcpy(path_bloque,configFile->puntoMontaje);
	string_append(&path_bloque, PATH_BLOQUES);

	string_append(&path_bloque, nro_bloque);
	string_append(&path_bloque, FORMATO_BIN);

	return path_bloque;
}

char *path_bloques(){
	char *path=(char *) malloc(1 + strlen(configFile->puntoMontaje) + strlen(PATH_BLOQUES) );
	strcpy(path,configFile->puntoMontaje);
	string_append(&path,MONTAJE_BLOQUES);
	return path;

}
