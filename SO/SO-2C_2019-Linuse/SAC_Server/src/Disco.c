#include "Disco.h"

void cargar_disco(void)
{

	diskSize = findSize(PATH_DISCO);
	int diskFD = open(PATH_DISCO, O_RDWR,0);
	myDisk = mmap(NULL,diskSize,PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED,diskFD,0);
	cargarHeader(myDisk);
	cargarBitmap(myDisk + 1);

}

void cargarHeader(GBlock *tempDisk)
{
	struct sac_header* tempHeader = (struct sac_header*) tempDisk;

	char* auxName = malloc(4);
	memset(auxName,0,4*sizeof(char));
	memcpy(auxName,tempHeader->sac,3);
	myBitmap = myDisk + 1;
	free(auxName);
}


long int findSize(char* file_name)
{

    FILE* fp = fopen(file_name, "r");
    if (fp == NULL) {
        printf("No existe el disco. Crear Disco y volver a iniciar el programa\n");
        return -1;
    }
    fseek(fp, 0L, SEEK_END);
    long int res = ftell(fp);
    fclose(fp);
    return res;
}


//----Bitmap----

void cargarBitmap(GBlock *tempDisk)
{
	tamanioBitmap = (diskSize/ BLOCK_SIZE /8) /BLOCK_SIZE;

	if (tamanioBitmap < 1)
	{
		tamanioBitmap = 1;
	}

	int cantidadBloques = diskSize / BLOCK_SIZE;

	bitarray = bitarray_create_with_mode(tempDisk, cantidadBloques/8, MSB_FIRST);

	size_t cantidadDebits = bitarray_get_max_bit (bitarray);
	int bloques_libres = cantidadDeBloquesLibres();
	int proximobloque= proximobloqueLibre();

	myNodeTable = myBitmap + tamanioBitmap;
	myBlocks = myNodeTable + CANT_MAX_ARCHIVOS;


}


int cantidadDeBloquesLibres (void){
	size_t	cantidadDebits= bitarray_get_max_bit (bitarray);
	int libre =ERROR;
	int i;
	for (i=0;i<cantidadDebits;i++){
		if (bitarray_test_bit(bitarray,i)==0){
			libre++;
		}
	}
	return libre;
}

int proximobloqueLibre (void){

	size_t	cantidadDebits= bitarray_get_max_bit (bitarray);
	int i;
	int libre= ERROR;
	pthread_mutex_lock(&mxBitmap);
	for (i=0;i<cantidadDebits;i++){
		if(bitarray_test_bit(bitarray,i)==0){
			libre=i;
			break;
		}
	}
	pthread_mutex_unlock(&mxBitmap);
	return libre;
}


//-------Tablas de nodos ---------------
void cargarTablaNodos(GBlock *tempDisk)
{
	//struct sac_header* tempTablaNodos = (struct sac_header*) tempDisk;

	int cantidadBloques = diskSize / tamanioBitmap -1 -1024;
	for(int i = 0; i < CANT_MAX_ARCHIVOS; i++){
		GFile *nodo = tempDisk +i;
		printf("\nNro. Bloque: %d     Bloque Padre:%d",nodo,nodo->parentBlock);
		printf("\nEstado: %d|   ",nodo->state);
		printf("Nombre Archivo: %s|   ",nodo->fname);
		printf("Tamaño archivo: %d|\n  ",nodo->file_size);
		printf("Bloques usados: [ ");
		for(int j = 0; j<= nodo->file_size/BLOCK_SIZE;j++ ){
			printf("%d  ",nodo->blocks[j]);
		}
		printf("]\n");
		printf("Fecha Creación: %llu Fecha Actualización: %llu",nodo->created, nodo->updated);
		printf("\n=====================================================================\n");
	}
}


