/*
 * funcionesMem.h
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_FUNCIONESMEM_H_
#define SRC_LIB_FUNCIONESMEM_H_


#include "globalesMem.h"
#include "sockets.h"

void* reservarMemoria(int size);
int obtenerSocketMaximoInicial();
int validar_recive(int status, int modo);
int validar_cliente(char *id);
int aceptarConexionEntranteDeKernel();
void * deserealizar(int head, void * buffer, int tamanio);
void remueve_salto_de_linea(char* salida, char* texto);
int ejecutar_comando(char* comando);
int existe_segmento(char* nombre_tabla);
void* reservarMemoria(int size);
int agregar_segmento_pagina(char* nombre_tabla, int key, char* valor,int bit_modificado);
int devolverMarcoLibre();
int buscar_pagina_libre();
int get_nro_ultimo_segmento();
void imprimir_bit_map();
int aplicar_protocolo_enviar(int fdReceptor, int head, void *mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);
int  ejecutar_comando_insert(char* nombre_tabla, int key, char* valor);
char* quitar_comillas(char* valor);
int  ejecutar_comando_insert(char* nombre_tabla, int key, char* valor);
pagina_t* get_valor_de_key_en_memoria(  int index_pagina, int index_segmento);
pagina_t* get_marco(int nro_marco);
unsigned long long obtener_timestamp();
char* ull_to_string(unsigned long long valor);
#endif /* SRC_LIB_FUNCIONESMEM_H_ */
