/*
 * lfc.c
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */


#include "lib/principalesLfs.h"

void main(void) {


	crearLoggerLFS();
	leerArchivoDeConfiguracion(RUTA_CONFIG_CLIENTE);
	leer_metaData_principal();
	iniciarEstructuras();
	//crearPuntoMontaje(); // Crea los directorios y ademas carga los puntos de montaje en variables globales
	//cargar_tablas_existentes();
	crearBitmap();
	crearHilos();
//	printf("Cantidad de bloques %d\n", config_MetaData.cantidad_bloques);
//	printf("tamaño del bloque %d\n", config_MetaData.tamanio_bloques);
//	printf("magic number %s\n", config_MetaData.magic_number);





}
