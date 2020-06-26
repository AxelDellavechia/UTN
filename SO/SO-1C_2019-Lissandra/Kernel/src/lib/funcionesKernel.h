/*
 * funcionesKernel.h
 *
 *  Created on: 8/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_FUNCIONESKERNEL_H_
#define SRC_LIB_FUNCIONESKERNEL_H_



#include "globalesKernel.h"
#include "sockets.h"

void* reservarMemoria(int size);
int validar_recive(int status, int modo);
char* comando_preparado(char* comando);
int ejecutar_comando(char* comando);
void remueve_salto_de_linea(char* salida, char* texto);
void error_parametros_faltantes();
int ejecutar_comando_run(char* ruta_script);
int agregar_entrada_tabla_pagina(int index_segmento, int bit_modificado);
#endif /* SRC_LIB_FUNCIONESKERNEL_H_ */
