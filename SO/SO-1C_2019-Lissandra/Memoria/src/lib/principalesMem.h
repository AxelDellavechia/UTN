/*
 * principalesMem.h
 *
 *  Created on: 12/4/2019
 *      Author: utnso
 */

#ifndef SRC_LIB_PRINCIPALESMEM_H_
#define SRC_LIB_PRINCIPALESMEM_H_



#include "funcionesMem.h"

void crearLoggerCliente();
archivoConfigMemoria *leerArchivoDeConfiguracion(char *ruta);
int conectarConLFS();
int esperar_y_AtenderScripts();
void handshake_cliente(int sockClienteDe, char *mensaje);
void* aplicar_protocolo_recibir(int fdEmisor, int* head);
void obtenerTamanioDeValue();
void iniciarEstructuras();
void consola();
void journal();
void actualizar_file_config();
void initGossiping();
#endif /* SRC_LIB_PRINCIPALESMEM_H_ */
