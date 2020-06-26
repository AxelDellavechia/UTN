/*
 * principalesKernel.h
 *
 *  Created on: 12/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_PRINCIPALESKERNEL_H_
#define SRC_LIB_PRINCIPALESKERNEL_H_




#include "funcionesKernel.h"

void crearLoggerKernel();
void leerArchivoDeConfiguracion(char *ruta);
void crearHilos();
void consola();
void consumidor();
void gossiping();
int conectarConMemoria();
void handshake_cliente(int sockClienteDe, char *mensaje);
void iniciarEstructuras();
void actualizar_info_tablas();
int conectar_to_memoria(int fdMem,int puerto, char* ip);
int generarHash(int key );
unsigned long long obtener_timestamp();
void logear_metricas();
void actualizar_file_config();
void load_config_file(char *ruta);
#endif /* SRC_LIB_PRINCIPALESKERNEL_H_ */
