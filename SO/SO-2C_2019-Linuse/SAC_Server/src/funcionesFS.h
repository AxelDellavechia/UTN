/*
 * funcionesFS.h
 *
 *  Created on: 5 dic. 2019
 *      Author: utnso
 */

#ifndef SRC_FUNCIONESFS_H_
#define SRC_FUNCIONESFS_H_

#include "Disco.h"
#include "Generales.h"
#include <pthread.h>

pthread_mutex_t mxNodos[1024];
pthread_mutex_t mxNodoLibre;
pthread_mutex_t mxTablaNodos;
pthread_mutex_t mxBitmap;
pthread_mutex_t mxReservaBloques;
pthread_mutex_t mxSocketsFD;
pthread_mutex_t mxEscriboNodo;
pthread_mutex_t mxHilos;


#endif /* SRC_FUNCIONESFS_H_ */
