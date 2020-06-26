#ifndef SRC_DISCO_H_
#define SRC_DISCO_H_

#include "Generales.h"
#include "Sockets.h"

#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/bitarray.h>

#include <string.h>

#define BLOCK_SIZE 4096
#define CANT_MAX_ARCHIVOS 1024
#define MAX_FILE_NAME 71
#define CANT_MAX_BLOQUES 1000
#define DIRECTORIO 2
#define ARCHIVO 1
#define BORRADO 0
#define PATH_DISCO "./disco.bin"


typedef struct sac_block{
	unsigned char bytes[BLOCK_SIZE];
}GBlock;

typedef struct  struct_pathRename{
	char* path;
	char* pathNew;
	uint32_t path_tam;
	uint32_t pathNew_tam;
} GPathRename;

typedef struct sac_header{
	unsigned char sac[3];
	uint32_t version;
	uint32_t bitmap_start;
	uint32_t bitmap_size;
	unsigned char padding[4081];
}Header;

typedef struct  struct_path{
	char* path;
	uint32_t path_tam;
} GPath;


typedef struct sac_tabla_nodos{
	uint8_t state;
	unsigned char fname[MAX_FILE_NAME];
	GBlock *parentBlock;
	uint32_t file_size;
	unsigned long long created;
	unsigned long long updated;
	GBlock *blocks[CANT_MAX_BLOQUES]; //son la cantidad de bloques que se pueden asignar para que cada registro de archivo pese 1 bloque
}GFile;

typedef struct sac_getattr{
	uint8_t state;
	uint32_t file_size;
	unsigned long long updated;
}GAttr;

typedef struct sac_write{
	char* path;
	char* contenido;
}GWrite;


typedef struct  struct_open{
	char* path;
	int path_tam;
	int create;
	int ensure;
	int trunc;
} GOpen;
long int findSize(char* file_name);


Header header;
t_bitarray* bitarray;
GBlock* myDisk;
GBlock* myBitmap;
GBlock* myNodeTable;
GBlock* myBlocks;
int diskFD;
int tamanioBitmap;
size_t diskSize;

#endif /* SRC_DISCO_H_ */
