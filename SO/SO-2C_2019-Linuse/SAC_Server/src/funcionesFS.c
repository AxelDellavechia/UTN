#include "funcionesFS.h"
#include "Disco.h"
#include <string.h>
#include <stdlib.h>

//int sac_mkdir(char* nameFile, GBlock *TablaNodo, GBlock *parent )
int sac_mkdir(GPath* path)
{
	printf("%s\n",path->path);
	int res= grabarTablaNodo(path->path,DIRECTORIO);
	if(res > ERROR){
		log_info(logger,"Directorio creado exitosamente");
		return res;
	}else{
		return res;
	}

}

int sac_rmdir(GPath* path){
	printf("%s\n",path->path);
	int totalFiles = cantFileName(path->path);
	int nodo = obtenerNumNodo(path->path);
	if( nodo != ERROR){
		if(dirIsEmpty(nodo)){ //si el directorio está vacío

		limpiarNodo(nodo);
		}else{
		return DIRECTORIO_NO_VACIO;
		}
	}else{
		log_info(logger,"El path no existe.");
		return ERROR;
	}
	log_info(logger,"Archivo borrado exitosamente");
return OK;
}

int dirIsEmpty(int nodo){
	GFile* nodoSeleccionado = myNodeTable;
	pthread_mutex_lock(&mxTablaNodos);
	for(int i = 0; i< CANT_MAX_ARCHIVOS; i++){
		if((nodoSeleccionado + i)->parentBlock == nodo && (nodoSeleccionado + i)->state != 0){
			pthread_mutex_unlock(&mxTablaNodos);
			return FALSE;
		}
	}
	pthread_mutex_unlock(&mxTablaNodos);
	return TRUE;
}

int sac_create(GPath* path)
{
	printf("%s\n",path->path);
	int res= grabarTablaNodo(path->path,ARCHIVO);
	if(res > ERROR){
		log_info(logger,"Archivo creado exitosamente");
		return res;
	}else{
		return res;
	}

}

int sac_mknod(GPath* path)
{
	printf("%s\n",path->path);
	int res= grabarTablaNodo(path->path,ARCHIVO);
	if(res > ERROR){
		log_info(logger,"Archivo creado exitosamente");
		return res;
	}else{
		return res;
	}

}


unsigned long long sac_utimes(GPath* path){
	printf("%s\n",path->path);
		unsigned long long timeUpdated;
		int totalFiles = cantFileName(path->path);
		int nodo = obtenerNumNodo(path->path);
		if( nodo != ERROR){
			pthread_mutex_lock(&mxNodos[nodo]);
			GFile *nodoSeleccionado = myNodeTable + nodo;
			timeUpdated = nodoSeleccionado->updated;

		}else{
			pthread_mutex_unlock(&mxNodos[nodo]);
			log_info(logger,"El path no existe.");
			return ERROR;
		}
		pthread_mutex_unlock(&mxNodos[nodo]);
		log_info(logger,"Se envían atributos.");
		return timeUpdated;
	}


int sac_open(GOpen *fileOpen)
{
	printf("%s\n",fileOpen->path);
	int result;
	int totalFiles = cantFileName(fileOpen->path);
	if(obtenerNumNodo(fileOpen->path) == NO_EXISTE_PATH){ //se fija si existe el archivo que quiere escribir
		if(fileOpen->create || (fileOpen->create && fileOpen->ensure))
		{
			result = sac_mknod(fileOpen->path);
		}

		log_info(logger,"Archivo abierto correctamente");
		return OK;
	}else{
		if(fileOpen->create && fileOpen->ensure){
			log_info(logger,"No existe el archivo");
			return ARCHIVO_YA_EXISTE;
		}
		return OK;
		}

}


int sac_write(GPath* path)
{
	printf("%s\n",path->path);
	char** data = string_split(path->path,"|");
	int totalFiles = cantFileName(data[0]);
	int nodo = obtenerNumNodo(data[0]);
	GFile *nodoSeleccionado = myNodeTable + nodo;
	if( nodo != ERROR)
	{ //se fija si existe el archivo que quiere escribir
		pthread_mutex_lock(&mxNodos[nodo]); //bloquea para que ya nadie pueda escribir o leer este nodo mientras se está modificando
		char *dataArchivo = string_new();
		int tamanio_a_grabar = 0;
		if(data[1]!= NULL){
			string_append(&dataArchivo, data[1]);
			 tamanio_a_grabar = strlen(dataArchivo);
			 free(dataArchivo);
		}else{
			return -1;
		}
		int bloquesNecesarios = 0;
		if((tamanio_a_grabar%BLOCK_SIZE) == 0){
			 bloquesNecesarios = tamanio_a_grabar/BLOCK_SIZE;
		}else{
			bloquesNecesarios = (tamanio_a_grabar/BLOCK_SIZE) + 1;
		}

		//teniendo el tamaño, calculo la cantidad de bloques que necesito y ya los reservo para que otro proceso no me los ocupe
		GBlock* bloquesReservados[bloquesNecesarios ];
		for(int i = 0;i< bloquesNecesarios; i++)
		{
			pthread_mutex_lock(&mxReservaBloques);
			bloquesReservados[i] = proximobloqueLibre();
			if(bloquesReservados[i] == ERROR)
			{
				while(i > 0){ //al fallar, libero todos los bloques ya asignados
					i--;
					bitarray_clean_bit(bitarray, bloquesReservados[i]);
				}
				pthread_mutex_unlock(&mxReservaBloques);
				pthread_mutex_unlock(&mxNodos[nodo]);
				log_info(logger,"No se pueden asignar bloques libres");
				for(int a = 0;a < 2;a++){
					free(data[a]);
				}
				free(data);

				return ERROR;
			}else{
				bitarray_set_bit(bitarray, bloquesReservados[i]);//lo marco como usado para que no me traiga de nuevo el mismo
				pthread_mutex_unlock(&mxReservaBloques);
			}

		}

		/*//no limpio los nodos, para que los agregue
		 * //Verifico si ya tiene bloques asignado ese nodo para limpiarlos y escribir los nuevos:
		if(nodoSeleccionado->file_size > 0){
			for(int i=0;i< CANT_MAX_BLOQUES && nodoSeleccionado->blocks[i] != 0; i++)
			{
				GBlock* bloque_a_limpiar = nodoSeleccionado->blocks[i];
				bitarray_clean_bit(bitarray,bloque_a_limpiar);
				nodoSeleccionado->blocks[i] = 0;
			}
		}*/

		int cantBloquesUsados = sizeof(nodoSeleccionado->blocks)/sizeof(nodoSeleccionado->blocks[0]);
		nodoSeleccionado->file_size = nodoSeleccionado->file_size  + tamanio_a_grabar;

		int desplazamiento = 0;
		for(int i = 0; i<bloquesNecesarios;i++)
		{
			if(tamanio_a_grabar >= BLOCK_SIZE)
			{//Si lo que voy a grabar ocupa mas de un bloque
				char* tempData = string_substring(data[1], desplazamiento,BLOCK_SIZE);
				grabarBloque(tempData,BLOCK_SIZE,bloquesReservados[i]);
				free(tempData);
				tamanio_a_grabar -= BLOCK_SIZE;
				desplazamiento += BLOCK_SIZE;
				nodoSeleccionado->blocks[cantBloquesUsados] = bloquesReservados[i];
				cantBloquesUsados++;
			}else
			{//Si lo que voy a grabar ocupa menos de un bloque
				char* tempData = string_substring(data[1], desplazamiento,BLOCK_SIZE);
				grabarBloque(tempData,tamanio_a_grabar,bloquesReservados[i]);
				free(tempData);
				tamanio_a_grabar -= BLOCK_SIZE;
				nodoSeleccionado->blocks[cantBloquesUsados] = bloquesReservados[i];
			}
		}
		nodoSeleccionado->updated = obtener_timestamp();
		pthread_mutex_unlock(&mxNodos[nodo]);
	
	}
		else{
			log_info(logger,"No existe el path");
			for(int a = 0;a < 2;a++){
				free(data[a]);
			}

		free(data);
		return ERROR;
	}
	log_info(logger,"Archivo escrito exitosamente.");
	for(int a = 0;a < 2;a++){
			free(data[a]);
	}
	free(data);
	return nodoSeleccionado->file_size;

}



int sac_read(char** respuesta,GPath* path){

	printf("%s\n",path->path);
	int totalFiles = cantFileName(path->path);
	char *lectura = string_new();
	//obtengo el nodo
	int nodo = obtenerNumNodo(path->path);

	if( nodo == ERROR){
		log_info(logger,"El path no existe.");
		free(lectura);
		return ERROR;
	}
	GFile *nodoSeleccionado = myNodeTable+ nodo;
	pthread_mutex_lock(&mxNodos[nodo]);
	int tamanio_a_leer = nodoSeleccionado->file_size;

	int bloquesUtilizados = 0;
	if((tamanio_a_leer%BLOCK_SIZE) == 0){
		bloquesUtilizados = tamanio_a_leer/BLOCK_SIZE;
	}else{
		bloquesUtilizados = (tamanio_a_leer/BLOCK_SIZE) + 1;
	}

	//int cantBloquesUsados = 0;
	for(int i = 0; i<bloquesUtilizados;i++)
	{
		//GBlock* bloque = ((tablaNodos + nodo-1)->blocks[i]);
		int bloqueGrabado = nodoSeleccionado->blocks[i];
		GBlock* bloque = myDisk + bloqueGrabado;

		int desplazamiento = 0;
		if(tamanio_a_leer >= BLOCK_SIZE)
		{//Si lo que voy a grabar ocupa mas de un bloque

			string_append(&lectura,bloque->bytes);
			tamanio_a_leer -= BLOCK_SIZE;
			desplazamiento += BLOCK_SIZE;
		//	cantBloquesUsados++;
		}else
		{//Si lo que voy a grabar ocupa menos de un bloque
			string_append(&lectura,bloque->bytes);
			tamanio_a_leer -= BLOCK_SIZE;

		}
		string_append(&lectura, "\0");
	}

	*respuesta = realloc(*respuesta, strlen(*respuesta) + strlen(lectura) + 1);
	string_append(respuesta,lectura);
	pthread_mutex_unlock(&mxNodos[nodo]);
	free(lectura);
	return OK;
}

int sac_readdir(char** respuesta, GPath* path)
{
//OJO QUE FALTA EL SEMAFORO!!!
	printf("%s\n",path->path);
	int totalFiles = cantFileName(path->path);
	char *lectura = string_new();
	int nodo = obtenerNumNodo(path->path);
	int contenidoPath = 0;
	pthread_mutex_lock(&mxTablaNodos);
	if( nodo != ERROR){
		//obtener parent y buscar archivos con ese parent
		GFile *nodoSeleccionado = myNodeTable;
		for(int i = 0;i<CANT_MAX_ARCHIVOS;i++){
			pthread_mutex_lock(&mxNodos[i]);
				if((nodoSeleccionado +i)->state == 0){
					pthread_mutex_unlock(&mxNodos[i]);
					continue;
				}
				if((nodoSeleccionado +i)->parentBlock == nodo){
					string_append(&lectura,(nodoSeleccionado + i)->fname);
					if(i+1 < CANT_MAX_ARCHIVOS){
					string_append(&lectura,"|");
					contenidoPath = OK;
					}
				}
				pthread_mutex_unlock(&mxNodos[i]);
			}
	}else{
		if(string_equals_ignore_case(path->path,"/") == TRUE || string_equals_ignore_case(path->path,"\0") == TRUE){
			GFile *nodoSeleccionado = myNodeTable;
					for(int i = 0;i<CANT_MAX_ARCHIVOS;i++){
							if((nodoSeleccionado +i)->state == 0){
								continue;
							}
							if((nodoSeleccionado +i)->parentBlock == NO_POSEE_PADRE){ //con -2 identifico a los que no tienen padre
								string_append(&lectura,(nodoSeleccionado + i)->fname);
								if(i+1 < CANT_MAX_ARCHIVOS){
								string_append(&lectura,"|");
								contenidoPath = OK;
								}

							}
						}

		}else{
		log_info(logger,"El path no existe.");
		pthread_mutex_lock(&mxTablaNodos);
		return ERROR;
		}
	}
	pthread_mutex_unlock(&mxTablaNodos);
	*respuesta = realloc(*respuesta, strlen(*respuesta) + strlen(lectura) + 1);
	string_append(respuesta,lectura);
	free(lectura);

	return contenidoPath;
}




int sac_unlink(GPath* path){
	printf("%s\n",path->path);
	int totalFiles = cantFileName(path->path);
	int nodo = obtenerNumNodo(path->path);
	if( nodo != ERROR){
		limpiarNodo(nodo);
	}else{
		log_info(logger,"El path no existe.");
		return ERROR;
	}
	log_info(logger,"Archivo borrado exitosamente");
return OK;
}



int sac_getattr( GPath* path, GAttr *ptrAttr){
	printf("%s\n",path->path);
	int totalFiles =  cantFileName(path->path);
	int nodo = obtenerNumNodo(path->path);
	if( nodo >=0 ){

			pthread_mutex_lock(&mxNodos[nodo]);
			GFile *nodoSeleccionado = myNodeTable + nodo;
			ptrAttr->file_size = nodoSeleccionado->file_size;
			ptrAttr->state = nodoSeleccionado->state;
			ptrAttr->updated = nodoSeleccionado->updated;
			pthread_mutex_unlock(&mxNodos[nodo]);

	}else{
		ptrAttr->file_size = 0;
		ptrAttr->state = DIRECTORIO;
		ptrAttr->updated = 0;
		log_info(logger,"El path no existe.");
		return ERROR;
	}

	log_info(logger,"Se envían atributos.");
	return OK;
}

int sac_rename(GPathRename path){


}


//ESTA FUNCION GRABA LA TABLA DE NODOS, SI FALLA DEVUELVE -1, SINO DEVUELVE EL NODO UTILIZADO
int grabarTablaNodo(char* path, int tipoArchivo){

	// recorro la tabla de nodos buscando un nodo libre:

		GBlock* parent = 0;
		int totalFiles = cantFileName(path);
		//char *ptrPath = path;
		char** nameFile = string_split(path,"/");
		char* nombreArchivo = string_new();
		int cont = 0;
		while(nameFile[cont]!= NULL){
			cont++;
		}
		if(cont==0){
			return ERROR;
		}

		string_append(&nombreArchivo,nameFile[cont-1]);
		 //si viene un solo archivo quiere decir que vaenel directorio raiz, entonces no tiene padre,
		//así que no necesito obtenerlo
		if (totalFiles > 1)
		{
			parent = obtenerNodoPadre(path);
		}else{
			parent = NO_POSEE_PADRE;
		}
		if (!string_is_empty(path))
		{
			if(parent != NO_EXISTE_PATH)
			{
				if(string_length(nombreArchivo) > MAX_FILE_NAME)
				{
					log_info (logger, "Nombre de archivo demasiado largo.");
					for(int a = 0;nameFile[a]!= NULL;a++){
							free(nameFile[a]);
					}
					free(nameFile);
					free(nombreArchivo);
					return NOMBRE_DEMASIADO_LARGO;
				}

				int nodo_libre = buscarNodoLibre(myNodeTable,tipoArchivo);

				if (nodo_libre == ERROR) {
					log_info (logger, "No hay mas nodos libres.");
					for(int a = 0;a < totalFiles;a++){
							free(nameFile[a]);
					}
					free(nameFile);
					free(nombreArchivo);
					return NO_HAY_MAS_NODOS_LIBRES;
				}
				pthread_mutex_lock(&mxNodos[nodo_libre]);
				GFile *nodoSeleccionado = myNodeTable + nodo_libre;
				if( verificarDuplicado(nombreArchivo,parent,tipoArchivo) == OK)
				{
					strcpy ((char*) nodoSeleccionado->fname, nombreArchivo);
					nodoSeleccionado->file_size = 0;
					nodoSeleccionado->parentBlock = parent;
					nodoSeleccionado->created = obtener_timestamp();
					nodoSeleccionado->updated = nodoSeleccionado->created;

					pthread_mutex_unlock(&mxNodos[nodo_libre]); //desbloquea el semaforo que venía bloqueado desde buscarNodoLibre
					char *log= string_new();
					string_append(&log,"Se grabó el archivo ");
					string_append(&log, nombreArchivo);
					log_info (logger, log);
					free (log);
					for(int a = 0;nameFile[a]!= NULL;a++){
							free(nameFile[a]);
					}
					free(nameFile);
					free(nombreArchivo);
					return OK;

				}else{
					log_info (logger, "Ya existe un archivo con ese nombre en ese directorio.");
					pthread_mutex_lock(&mxEscriboNodo);
					nodoSeleccionado->state = 0; //Ya que no se pudo grabar, seteo el nodo para que vuelva a estar disponible.
					pthread_mutex_unlock(&mxEscriboNodo);
					pthread_mutex_unlock(&mxNodos[nodo_libre]); //desbloquea el semaforo
					for(int a = 0;nameFile[a]!= NULL;a++){
							free(nameFile[a]);
					}
					free(nameFile);
					free(nombreArchivo);
					return ARCHIVO_YA_EXISTE;

				}

			}else{
				log_info (logger, "No existe el path especificado");
				for(int a = 1;nameFile[a]!= NULL;a++){
						free(nameFile[a]);
				}
				free(nameFile);
				free(nombreArchivo);
				return NO_EXISTE_PATH;

			}
		}else{
			log_info(logger, "Path vacío");
			for(int a = 0;nameFile[a]!= NULL;a++){
					free(nameFile[a]);
			}
			free(nameFile);
			return PATH_VACIO;

		}
		for(int a = 0;nameFile[a]!= NULL;a++){
				free(nameFile[a]);
		}
		free(nameFile);
		free(nombreArchivo);
		return OK;
}



int buscarNodoLibre (GBlock *TablaNodo,int state)
{
//	pthread_mutex_lock(&mxTablaNodos);
	pthread_mutex_lock(&mxNodoLibre); //bloqueo para que no exista condición de carrera y se le asigne el mismo nodo a otro hilo
	GFile *nodo = myNodeTable;
	int nodoDisponible = 0;
	//recorro todos los nodos, buscando los que tengan estado = 0, que significa que están disponibles
	int check = 0;

	while (nodoDisponible < CANT_MAX_ARCHIVOS && check == 0 )
	{
		pthread_mutex_lock(&mxNodos[nodoDisponible]);
		if((nodo + nodoDisponible)->state == 0){
			check = 1;
			pthread_mutex_unlock(&mxNodos[nodoDisponible]);
		}else{
			pthread_mutex_unlock(&mxNodos[nodoDisponible]);
			nodoDisponible++;
		}

	}

	if(nodoDisponible >= CANT_MAX_ARCHIVOS)
	{
		pthread_mutex_unlock(&mxNodoLibre);
//		pthread_mutex_unlock(&mxTablaNodos);
		return ERROR;
	}
	pthread_mutex_lock(&mxNodos[nodoDisponible]);
	pthread_mutex_lock(&mxEscriboNodo);
	(nodo + nodoDisponible)->state = state; //Ya marca el nodo como ocupado para que si otra función pide un nodo libre, no le asigne este
	pthread_mutex_unlock(&mxEscriboNodo);
	pthread_mutex_unlock(&mxNodos[nodoDisponible]);
	pthread_mutex_unlock(&mxNodoLibre);
	//pthread_mutex_unlock(&mxTablaNodos);
	return nodoDisponible;
}


/*
*Esta función verifica si el path donde se desea crear un archivo existe.
*Y devuelve cual sería el padre del archivo que se va a crear o leer.
*Y En caso de que se vaya a leer un archivo ya existente, devuelve el nodo
*Se le pasa por parámetro el campo función, que indica si se va a crear un archivo nuevo (funcion = 1)
*o solo leerlo (funcion = 0)
*/


int obtenerNodoPadre(char* path){
	int ultimoCampo = 0;
	if(string_equals_ignore_case(path,"/") != TRUE){
			int acum=0;
			char *ptrPath = path;
			for(int i=1; i< string_length(path); i++)
			{
				if(*(ptrPath)== '/')
				{
					   acum =i;
				 }
			ptrPath = ptrPath + 1;
			}

			char* pathParent = string_substring_until(path,acum-1);
			int nodo =obtenerNumNodo(pathParent);
			free(pathParent); //si devuelve -1 es que no existe
			return nodo;
		}else{
			return PARENT_RAIZ;
		}

}

int obtenerNumNodo(char* path){
	GFile *nodo = myNodeTable;

	int numNodo = 0;

	char** nameFiles = string_split(path,"/");
	char* nombreArchivo = string_new();
	int cont = 0;
	if(string_is_empty(path) == TRUE){
		for(int a = 0;nameFiles[a]!=NULL;a++){
				free(nameFiles[a]);
		}
		free(nameFiles);
		return PARENT_RAIZ;
	}

	while(nameFiles[cont]!= NULL){
		cont++;
	}
	if(cont==0){
		for(int a = 0;nameFiles[a]!=NULL;a++){
				free(nameFiles[a]);
		}
		free(nombreArchivo);
		free(nameFiles);
		return ERROR;
	}
	else{
		string_append(&nombreArchivo,nameFiles[cont-1]);
		int i = 0;
		pthread_mutex_lock(&mxTablaNodos);
		while(nameFiles[i] != NULL){

			int j = 0;
			int check = FALSE;
			//voy recorriendo todos los nodos verificando si el nombre existe. Si el nodo a analizar está en 0 lo ignoro porque se supone que está borrado

			while (j < CANT_MAX_ARCHIVOS &&  check != TRUE)
				{
					char *tempName = string_new();
					pthread_mutex_lock(&mxNodos[j]);
					string_append(&tempName, (nodo + j)->fname);
					pthread_mutex_unlock(&mxNodos[j]);
					check = string_equals_ignore_case(tempName,nombreArchivo);
					free(tempName);
					j++;
				}
			if(j >= CANT_MAX_ARCHIVOS  || (nodo + j -1)->state == BORRADO)
			{
				numNodo = NO_EXISTE_PATH;
			}else
			{
				//parent = myNodeTable + j;//  Le asigno el puntero al bloque padre
				numNodo = j -1;
			}

		i++;
		}
		pthread_mutex_unlock(&mxTablaNodos);
	}
for(int a = 0;nameFiles[a]!=NULL;a++){
		free(nameFiles[a]);
}
free(nameFiles);
free(nombreArchivo);
return numNodo;
}


/*int verificarPath(char* path){
	GFile *nodo = myNodeTable;

	int numNodo = 0;

	char** nameFiles = string_split(path,"/");
	char* nombreArchivo = string_new();
	int cont = 0;
	while(nameFiles[cont]!= NULL){
		cont++;
	}
	if(cont==0){
		return ERROR;
	}
	if(string_equals_ignore_case(path,"/") != TRUE || cont == 1){
				return PARENT_RAIZ;

	}else{

		string_append(&nombreArchivo,nameFiles[cont]);
		int j = 0;
		bool check = FALSE;
		//voy recorriendo todos los nodos verificando si el nombre existe. Si el nodo a analizar está en 0 lo ignoro porque se supone que está borrado
		pthread_mutex_lock(&mxTablaNodos);
		while (j < CANT_MAX_ARCHIVOS &&  check != TRUE)
			{
				char *tempName = string_new();
				pthread_mutex_lock(&mxNodos[j]);
				string_append(&tempName, (nodo + j)->fname);
				pthread_mutex_unlock(&mxNodos[j]);
				check = string_equals_ignore_case(tempName,nombreArchivo);
				free(tempName);
				j++;
			}
		if(j >= CANT_MAX_ARCHIVOS  || (nodo + j -1)->state == BORRADO)
		{
			numNodo = NO_EXISTE_PATH;
		}else
		{
			//parent = myNodeTable + j;//  Le asigno el puntero al bloque padre
			numNodo = j -1;
		}
		pthread_mutex_unlock(&mxTablaNodos);
	}
return numNodo;
}
*/
/*
int verificarPath(char* path, int totalFiles, int funcion)
{
	GFile *nodo = myNodeTable;

	int parent = 0;

	char** nameFiles = string_split(path,"/");
	char* nombreArchivo = string_new();
			int cont = 0;
			while(nameFiles[cont]!= NULL){
				cont++;
			}
			if(cont==0){
				return ERROR;
			}
	string_append(&nombreArchivo,nameFiles[cont-1-funcion]);
	pthread_mutex_lock(&mxTablaNodos);
	if(string_equals_ignore_case(path,"/") == TRUE){
		pthread_mutex_unlock(&mxTablaNodos);
		return PARENT_RAIZ;
	}
	if(totalFiles == 2 && funcion == CREAR){
		pthread_mutex_unlock(&mxTablaNodos);
		return PARENT_RAIZ;
	}

	if(string_equals_ignore_case(path,"/") != TRUE){
				if(nombreArchivo!=NULL){
					int j = 0;
					bool check = FALSE;
					//voy recorriendo todos los nodos verificando si el nombre existe. Si el nodo a analizar está en 0 lo ignoro porque se supone que está borrado
					while (j < CANT_MAX_ARCHIVOS &&  check != TRUE)
						{
							char *tempName = string_new();
							pthread_mutex_lock(&mxNodos[j]);
							string_append(&tempName, (nodo + j)->fname);
							pthread_mutex_unlock(&mxNodos[j]);
							check = string_equals_ignore_case(tempName,nombreArchivo );
							free(tempName);
							j++;
						}

					if(j >= CANT_MAX_ARCHIVOS  || (nodo + j -1)->state == BORRADO)
						{
						parent = NO_EXISTE_PATH;
					}else
					{
						//parent = myNodeTable + j;//  Le asigno el puntero al bloque padre
						parent = j -1;
					}
				}

			for(int a = 0;nameFiles[a]!=NULL;a++){
				free(nameFiles[a]);
			}
	}else{
		parent = ERROR;
	}
	pthread_mutex_unlock(&mxTablaNodos);
	free(nameFiles);
	return parent;
}*/

/*int verificarPath(char* path, int totalFiles, int funcion)
{
	GFile *nodo = myNodeTable;

	int parent = 0;

	char** nameFiles = string_split(path,"/");
	char* nombreArchivo = string_new();
			int cont = 0;
			while(nameFiles[cont]!= NULL){
				cont++;
			}
			if(cont==0){
				return ERROR;
			}
	string_append(&nombreArchivo,nameFiles[cont-1-funcion]);
	pthread_mutex_lock(&mxTablaNodos);
	if(string_equals_ignore_case(path,"/") == TRUE){
		pthread_mutex_unlock(&mxTablaNodos);
		return PARENT_RAIZ;
	}
	if(totalFiles == 2 && funcion == CREAR){
		pthread_mutex_unlock(&mxTablaNodos);
		return PARENT_RAIZ;
	}

	if(string_equals_ignore_case(path,"/") != TRUE){
		for(int i = 0 ; i< totalFiles - funcion && nameFiles[i]!= NULL; i++)
			{
				if(nameFiles[i]!=NULL){
					int j = 0;
					bool check = FALSE;
					//voy recorriendo todos los nodos verificando si el nombre existe. Si el nodo a analizar está en 0 lo ignoro porque se supone que está borrado
					while (j < CANT_MAX_ARCHIVOS &&  check != TRUE)
						{
							char *tempName = string_new();
							pthread_mutex_lock(&mxNodos[j]);
							string_append(&tempName, (nodo + j)->fname);
							pthread_mutex_unlock(&mxNodos[j]);
							check = string_equals_ignore_case(tempName,nameFiles[i] );
							free(tempName);
							j++;
						}

					if(j >= CANT_MAX_ARCHIVOS  || (nodo + j -1)->state == BORRADO)
						{
						parent = NO_EXISTE_PATH;
					}else
					{
						//parent = myNodeTable + j;//  Le asigno el puntero al bloque padre
						parent = j -1;
					}
				}
			}

			for(int a = 0;nameFiles[a]!=NULL;a++){
				free(nameFiles[a]);
			}
	}else{
		parent = ERROR;
	}
	pthread_mutex_unlock(&mxTablaNodos);
	free(nameFiles);
	return parent;
}*/

int cantFileName(char* path)
{
	int tamanio = 0;
	char *ptrPath = path;
	tamanio =string_length(path);
	if (tamanio < 1)
	{
		return 0;
	}

	if(string_equals_ignore_case(path,"/") != TRUE){
		int acum=1;
			for(int i=0; i< string_length(path); i++)
			{

			//if(*(ptrPath)=='\\')
			if(*(ptrPath)== '/')
			{
				   acum++;
			 }
			ptrPath = ptrPath + 1;
			}
			return acum;
	}else{
		return 1;
	}

}

int verificarDuplicado(char* fileName,GBlock* parent, int state){
	GFile *nodo = myNodeTable;

	//voy recorriendo todos los nodos verificando si el nombre existe y en caso de que existiera tengan al mismo bloque padre
	for (int j = 0; j < CANT_MAX_ARCHIVOS; j++)
	{
		if(string_equals_ignore_case((nodo + j)->fname,fileName) == TRUE  && (nodo + j)->parentBlock == parent && (nodo + j)->state == state)
		{
			return ERROR;
		}

	}
return OK;

}


int grabarBloque(char* data,int tamanio_a_grabar, int bloque)
{
	//struct sac_block* tempBloque = (struct sac_block*) myBlocks;
	GBlock *tempBloque = myDisk + bloque;

	strcpy ((char*) tempBloque->bytes, data);
	//printf("se va a grabar %s \n",data);

	return string_length(data);
}


void limpiarNodo(int nodo){
	pthread_mutex_lock(&mxNodos[nodo]);
	GFile *nodoSeleccionado = myNodeTable + nodo;

	nodoSeleccionado->state = BORRADO;
	nodoSeleccionado->parentBlock = NO_POSEE_PADRE;
	if((nodoSeleccionado)->file_size > 0){
		(nodoSeleccionado)->file_size = 0;
		for(int i=0;i< CANT_MAX_BLOQUES && nodoSeleccionado->blocks[i] != 0; i++)
		{
			GBlock* bloque_a_limpiar = nodoSeleccionado->blocks[i];
			bitarray_clean_bit(bitarray,bloque_a_limpiar);
			nodoSeleccionado->blocks[i] = 0;
		}
	}
	(nodoSeleccionado)->created = 0;
	(nodoSeleccionado)->updated = 0;
	strcpy ((char*) (nodoSeleccionado)->fname, "\0");
	pthread_mutex_unlock(&mxNodos[nodo]);

}




