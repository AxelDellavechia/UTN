#include "src/Generales.h"

int main(int argc, char *argv[]) {

	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	crearLogger();

	configSacCli = reservarMemoria(sizeof(archivoConfigSacCli));

	fdSacServer = nuevoSocket();

	fdSacCli = nuevoSocket() ;

	inicializar_semaforos();

	log_info(logger, "Por setear los valores del archivo de configuracion");

	leerArchivoDeConfiguracion(RUTA_CONFIG_MEM,logger);

	asociarSocket(fdSacCli, configSacCli->puertoEscucha);

	escucharSocket(fdSacCli, CONEXIONES_PERMITIDAS);

	signal (SIGINT, capturarError);

	return fuse_main(args.argc, args.argv, &sacCli_oper,NULL);

}
