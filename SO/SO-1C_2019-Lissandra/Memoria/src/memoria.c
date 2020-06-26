/*
 * memoria.c
 *
 *  Created on: 7/4/2019
 *      Author: utnso
 */



#include "lib/principalesMem.h"

int main(void) {

	crearLoggerMemoria();
	leerArchivoDeConfiguracion(RUTA_CONFIG_MEM);

	if (conectarConLFS()){ // Conexión con LFS
		handshake_cliente(fdLfs, "M");

		obtenerTamanioDeValue();
		iniciarEstructuras();

		pthread_t hilo_consola;
		pthread_t hilo_journal;
		pthread_t hilo_inotify;
		pthread_t hilo_gossiping;

		pthread_create(&hilo_consola, NULL, (void*) consola, NULL);
		pthread_create(&hilo_journal, NULL, (void*) journal, NULL);
		pthread_create(&hilo_inotify, NULL, (void*) actualizar_file_config, NULL);
		pthread_create(&hilo_gossiping, NULL, (void*) initGossiping, NULL);

		//Pedir al FS los datos requeridos esto incluye  el tamaño del value
		//Iniciar las estructuras de la memoria principal(Tablas)
		//Iniciar el proceso de gossiping

		esperar_y_AtenderScripts();

		pthread_join(hilo_consola, NULL);
		pthread_join(hilo_journal, NULL);
		pthread_join(hilo_inotify, NULL);
		pthread_join(hilo_gossiping, NULL);
		pthread_mutex_destroy(&mutex_socket_lfs);
	}


}
