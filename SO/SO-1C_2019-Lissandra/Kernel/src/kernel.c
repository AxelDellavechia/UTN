/*
 * kernel.c
 *
 *  Created on: 8/4/2019
 *      Author: utnso
 */


#include "lib/principalesKernel.h"

int main(void) {

	crearLoggerKernel();
	leerArchivoDeConfiguracion(RUTA_CONFIG_KERNEL);
	iniciarEstructuras();
	crearHilos();

	return EXIT_SUCCESS;
}

