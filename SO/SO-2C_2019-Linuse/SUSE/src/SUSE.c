/*
 ============================================================================
 Name        : SUSE.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include "hilolay/hilolay.h"
#include "../libSUSE/src/Generales.h"

int main(void) {
	crearLoggerSUSE();
	leerArchivoDeConfiguracion(RUTA_CONFIG_SUSE);
	iniciarEstructuras();
	servidor();
	//return EXIT_SUCCESS;
}
