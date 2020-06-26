/*
 * funcionesLfs.h
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_FUNCIONESLFS_H_
#define SRC_LIB_FUNCIONESLFS_H_

#include "globalesLfs.h"
#include "sockets.h"

void* reservarMemoria(int size);
int validar_conexion(int ret, int modo);
int validar_recive(int status, int modo);
int validar_cliente(char *id);
void * deserealizar(int head, void * buffer, int tamanio);
void *elegirFuncion(protocolo head);
int ejecutar_comando(char* comando);
char* comando_preparado(char* comando);
void remueve_salto_de_linea(char* salida, char* texto);
unsigned long long buscar_en_memtable(char* nombre_tabla, int key, char* valor);
unsigned long long buscar_en_temporales(char* nombre_tabla, int key, char* valor);
unsigned long long buscar_en_bin(char* nombre_tabla, int key, int cant_particiones, char* valor);
void leer_metadata (char* *nombre_tabla);
void ejecutar_comando_describe(char* nombre_tabla);
void cargar_tablas_existentes(void);
char* armar_respuesta_describe(t_list* lista_resultados);
void ejecutar_comando_describe_para_memoria(char* query, int sockMemoria);
t_list* get_resultado_describe(char* nombre_tabla);
char *path_metadata(char* nombre_tabla);
char *path_bitmap();
char* path_punto_montaje_tables();
int agregar_tabla(char* nombre_tabla,char* consistency, int cant_partitions, int compaction_time );
t_config_MetaData_tabla* leer_metaData_tabla(char* nombre_tabla);
unsigned long long obtener_timestamp();
char* ull_to_string(unsigned long long valor);
char* path_directorio_tabla(char* nombre_tabla);
char* path_file(char* nombre_tabla, char* nombre_archivo);
char* path_bloque(char* nro_bloque);
void quitar_corchetes(bin_tmp_t* file);
char * get_content_archivo(char * path_bloque, int size);
bin_tmp_t* leer_bin_tmp_tmpc(char* path);
char *path_bloques();
void ejecutar_dump(void);
unsigned long long buscar_en_temporales_y_binarios(char* nombre_tabla, int key, char* valor);
#endif /* SRC_LIB_FUNCIONESLFS_H_ */
