/*
 * principalesLfs.h
 *
 *  Created on: 12/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_PRINCIPALESLFS_H_
#define SRC_LIB_PRINCIPALESLFS_H_

#include "funcionesLfs.h"


void crearLoggerLFS();
void leerArchivoDeConfiguracion(char *ruta);
void crearHilos();
void consola();
void servidor();
void crearHiloQueAtiendeAMemoria(int *sockCliente);
void* memoria(void* fdCliente);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);
void enviarTamanioValue(int fd, int tamanio);
void iniciarEstructuras();
int creacionDeArchivoBitmap(char *path,int cantidad);
void crearBitmap();
int leerMinetaData();
int leer_metaData_principal();
void dump();
void actualizar_file_config();
#endif /* SRC_LIB_PRINCIPALESLFS_H_ */
