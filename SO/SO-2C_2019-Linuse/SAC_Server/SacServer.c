#include "src/Generales.h"
#include "src/Disco.h"


int main(void) {


 	crearLogger();
	leerArchivoDeConfiguracion(RUTA_CONFIG_MEM);
	cargar_disco();
	inicializar_semaforos();
	crearHilos();

}

