/*
 * funcionesKernel.c
 *
 *  Created on: 8/4/2019
 *      Author: utnso
 */



#include "funcionesKernel.h"


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
	if(!strcmp(id, "M")) {
		log_info(logger, "Servidor aceptado.");
		return TRUE;
	} else {
		printf("Servidor rechazado.\n");
		return FALSE;
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

int memoria_existe_en_ec(int nro_mem){
	int i;
	for (i = 0; i < queue_size(cola_ec); i++){
		//memoria_t* mem = (memoria_t*) queue_pop(cola_ec);
		memoria_t* mem = (memoria_t*) list_get(cola_ec->elements,i);
		if(mem->memory_number == nro_mem){
			return -1;
		}
	}

	return 1;
}

int ejecutar_comando_add(int nro_memoria, char* consistency){

	bool esMemoria(memoria_t* mem){return mem->memory_number == nro_memoria;}
	memoria_t* memoria_encontrada = list_find(lista_memorias,(void*) esMemoria);
	if(memoria_encontrada == NULL){
		printf("El number memory no se encuentra en el sistema en este momento\n");
		return EXIT_FAILURE;
	}


	memoria_t* memoria_copia = malloc(sizeof(memoria_t));
	int tam_ip = strlen(memoria_encontrada->ip) + 1;
	memoria_copia->ip = malloc(tam_ip);
	strcpy(memoria_copia->ip, memoria_encontrada->ip);
	memoria_copia->puerto = memoria_encontrada->puerto;
	memoria_copia->memory_number = memoria_encontrada->memory_number;

	string_to_upper(consistency);

	if(strcmp(consistency,"SC") == 0){


		memoria_t* mem_busq = list_find(lista_sc,(void*) esMemoria);
		if(mem_busq == NULL){
			if(list_size(lista_sc) == 0){
				list_add(lista_sc,memoria_copia);
			}else{
				free(memoria_copia);
				printf("El criterio SC ya tiene asignada una memoria. Solo puede tener una memoria\n");
				return EXIT_FAILURE;
			}
		}else{
			free(memoria_copia);
			printf("La memoria ingresada ya se encuentra asignado al criterio ingresado\n");
			return EXIT_FAILURE;
		}

	}

	if(strcmp(consistency,"SHC") == 0){
		memoria_t* mem_busq = list_find(lista_shc,(void*) esMemoria);
		if(mem_busq == NULL){
			list_add(lista_shc,memoria_copia);
		}else{
			free(memoria_copia);
			printf("La memoria ingresada ya se encuentra asignado al criterio ingresado\n");
			return EXIT_FAILURE;
		}

	}

	if(strcmp(consistency,"EC") == 0){

		if(memoria_existe_en_ec(memoria_copia->memory_number) > 0) {
			queue_push(cola_ec,memoria_copia);
		}else{
			free(memoria_copia);
			printf("La memoria ingresada ya se encuentra asignado al criterio ingresado\n");
			return EXIT_FAILURE;
		}



	}


}


void ejecutar_comando_sc(){
	int i;
	for(i=0;i<list_size(lista_sc);i++){
		memoria_t* mem = list_get(lista_sc,i);
		printf("NRO MEMORIA: %i\n", mem->memory_number);
		printf("IP: %s\n", mem->ip);
		printf("PUERTO: %i\n\n", mem->puerto);

	}
}


void ejecutar_comando_shc(){
	int i;
	for(i=0;i<list_size(lista_shc);i++){
		memoria_t* mem = list_get(lista_shc,i);
		printf("NRO MEMORIA: %i\n", mem->memory_number);
		printf("IP: %s\n", mem->ip);
		printf("PUERTO: %i\n\n", mem->puerto);

	}
}


void ejecutar_comando_ec(){
	int i;
	for(i=0;i<queue_size(cola_ec);i++){
		memoria_t* mem = list_get(cola_ec->elements,i);
		printf("NRO MEMORIA: %i\n", mem->memory_number);
		printf("IP: %s\n", mem->ip);
		printf("PUERTO: %i\n\n", mem->puerto);

	}
}

void ejecutar_comando_tablas(){

	int z;
	for(z=0;z<list_size(lista_tablas);z++){
		t_config_MetaData_tabla* tab = list_get(lista_tablas,z);

			printf("nombre tabla %s\n", tab->nombre_tabla);
			printf("consistencia %s\n", tab->consistency);
			printf("compactacion %i\n", tab->compaction_time);
			printf("particiones %i\n\n\n", tab->partitions);
	}
}

void ejecutar_comando_memorias(){

	int z;
	for(z=0;z<list_size(lista_memorias);z++){
		memoria_t* mem = list_get(lista_memorias,z);

			printf("Nro Memoria %i\n", mem->memory_number);
			printf("IP %s\n", mem->ip);
			printf("Puerto %i\n\n\n", mem->puerto);

	}
}

void crear_pcb(char* query){

	pcb_t* script = malloc(sizeof(pcb_t));

	int tam_query = strlen(query);
	script->instruciones[0] = malloc(tam_query + 1);
	strcpy(script->instruciones[0],query);
	//printf("script->instruciones[z]: %s\n",script->instruciones[0]);
	script->ip = 0;
	script->cant_instrucciones = 1;

	pthread_mutex_lock(&mutex_cola_listos);
	queue_push(cola_listos,script);
	pthread_mutex_unlock(&mutex_cola_listos);
	sem_post(&contador_cola_listos);
}

int ejecutar_comando(char* comando) {
	int ret = 0;
	char* comando_listo = comando_preparado(comando);
	char** parametros = string_n_split(comando_listo, 6, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_AYUDA)) {
		printf("Se ejecuto comando ayuda\n");
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_SELECT)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_INSERT)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_CREATE)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DESCRIBE)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DROP)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_JOURNAL)) {
		crear_pcb(comando_listo);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_ADD)) {
		ejecutar_comando_add(atoi(parametros[2]),parametros[4]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_RUN)) {
		if (parametros[1] == NULL ) {
			error_parametros_faltantes();
			return 1;
		}
		ret = ejecutar_comando_run(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_METRICS)) {
		mostrar_metricas_consola();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_SC)) {
		ejecutar_comando_sc();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_SHC)) {
		ejecutar_comando_shc();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_EC)) {
		ejecutar_comando_ec();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_TABLAS)) {
		ejecutar_comando_tablas();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_MEMORIAS)) {
		ejecutar_comando_memorias();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_GOSSIPING)) {
		exec_gossiping();
		return ret;

	}else if (string_equals_ignore_case(parametros[0], COMANDO_T_GOSSIPING)) {
		mostrar_tabla_gossiping();
		return ret;

	}

	else {
		log_error(logger,"Comando incorrecto ingrese ayuda para una lista de comandos validos o salir para cerrar el sistema");
		return 1;
	}
	return 0;
}


char* getNombreScript(char* primerParametro)
{
	char* scriptName = primerParametro;
	if (string_starts_with(primerParametro, "./"))
		scriptName = string_substring(primerParametro, 2, strlen(primerParametro));
	return scriptName;
}


short getTamanioArchivoScript(char * nombreArchivo, int* cant_instruc){


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

//short getTamanioArchivoScript(char * nombreArchivo){
//
//	//char* nombreScript=getNombreScript(nombreArchivo);
//	char* nombreScript=nombreArchivo;
//	FILE *archivoScript = fopen(nombreScript, "r");
//	if (archivoScript == NULL)
//	{
//		printf("Error de apertura en script :%s\n\n",nombreScript);
//		fclose(archivoScript);
//		return 0;
//	}
//
//
//
//	short i = 0;
//	while (!feof(archivoScript))
//	{
//		fgetc(archivoScript);
//		i++;
//	}
//	fclose(archivoScript);
//	return i;
//}


char * getContentArchivoScript(char * nombreArchivo, int size)
{

	char *cadenaRetorno;
	cadenaRetorno = malloc(size + 1);
	strcpy(cadenaRetorno, "");


	char* nombreScript = nombreArchivo;

	FILE * archivoScript = fopen(nombreScript, "r");
	if (archivoScript == NULL)
	{
		printf("Error de apertura en script:%s\n\n",nombreScript);
		return "\0";
	}
	else
	{	/*Recorro el script y lo voy anexando linea por linea*/
			do
			{
				char cadena[MAX_LINEA];
				fgets(cadena,MAX_LINEA,archivoScript);
				strcat(cadenaRetorno,cadena);
			}
			while ( !feof(archivoScript) && (strlen(cadenaRetorno)<size) );

			fclose(archivoScript);
	}


	return cadenaRetorno;
}

int ejecutar_comando_run(char* ruta_script) {


	char *programaScript;

	/*OBTENGO EL TAMAÑO DEL ARCHIVO DE SCRIPT */
	int tamanioArchivo = 0;
	int* cant_instrucciones = malloc(INT);
	if (!(tamanioArchivo = getTamanioArchivoScript(ruta_script, cant_instrucciones) -1))
		return EXIT_FAILURE;


	/*PASO EL CONTENIDO DEL ARCHIVO DE PROGRAMA ANSISOP A UN STRING*/
	programaScript = getContentArchivoScript(ruta_script, tamanioArchivo);


	if (!strcmp(programaScript, "\0"))
		return EXIT_FAILURE;




	char** querys = string_n_split(programaScript, *cant_instrucciones, "\n");


	//printf("cant_instrucciones: %i\n",*cant_instrucciones);

	pcb_t* script = malloc(sizeof(pcb_t));
	int z;
	for(z=0;z<(*cant_instrucciones);z++){
		int tam_query = strlen(querys[z]);
		//printf("tam_query: %i : %i",z,tam_query);
		script->instruciones[z] = malloc(tam_query + 1);
		memset(script->instruciones[z],'\0',tam_query + 1);
		strcpy(script->instruciones[z],querys[z]);
		//printf("script->instruciones %i =  %s\n",z,script->instruciones[z]);
	}


	script->ip = 0;
	script->cant_instrucciones = *cant_instrucciones;

	printf("INST 0: %s\n",script->instruciones[0]);
	printf("INST 1: %s\n",script->instruciones[1]);
//	int h;
//	for(h=0;h<script->cant_instrucciones;h++){
//
//		printf("ZZZZZZZZ: %i =  %s\n",h,script->instruciones[h]);
//	}


	pthread_mutex_lock(&mutex_cola_listos);
	queue_push(cola_listos,script);
	//printf("tam de cola listos:  %i\n", queue_size(cola_listos));
	pthread_mutex_unlock(&mutex_cola_listos);
	sem_post(&contador_cola_listos);

	//printf("%s\n", programaScript);
	//printf("cant_instrucciones: %i\n",*cant_instrucciones);
	//printf("script->instruciones: %s\n", script->instruciones);


	free(programaScript);
	free(cant_instrucciones);
	free(ruta_script);
	return EXIT_SUCCESS;
}
void error_parametros_faltantes() {
	printf("Falta ingresar parametros\n");
}

/************************PROTOCOLO DE COMUNICACION**************************************/


// SEREALIZAR: Del mensaje listo para enviar, al buffer
void * serealizar(int head, void * mensaje, int tamanio){

	void * buffer = NULL;

	switch(head) {
	// CASE 1: El mensaje es un texto (char*)
	case DESCRIBE: case CREATE: case INSERT: case SELECT: case JOURNAL: case DROP:{
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
	case TAMANIO_VALUE: case DESCRIBE: case CREATE: case TABLA_GOSSIPING: case INSERT: case SELECT: case JOURNAL: case DROP:{
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
			case DESCRIBE: case CREATE: case INSERT: case SELECT: case JOURNAL: case DROP:{
				tamanio = strlen((char*) mensaje) + 1;
				//printf("El tamanio de query es %i\n",tamanio);
				break;
			}
			case TABLA_GOSSIPING:{
				tamanio = 0;
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
