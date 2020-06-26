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
archConfigMUSE *leerArchivoDeConfiguracion(char *ruta);
int conectarConLFS();
int GestionarSolicitudes();
void atender_pedido();
void handshake_cliente(int sockClienteDe, char *mensaje);
void CrearLoggerMemoria();
void IniciarEstructuras();
#endif /* SRC_LIB_PRINCIPALESMEM_H_ */
