#include <stddef.h>
#include <stdlib.h>
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


/* Este es el contenido por defecto que va a contener
 * el unico archivo que se encuentre presente en el FS.
 * Si se modifica la cadena se podra ver reflejado cuando
 * se lea el contenido del archivo
 */
#define DEFAULT_FILE_CONTENT "Hello World!\n"

/*
 * Este es el nombre del archivo que se va a encontrar dentro de nuestro FS
 */
#define DEFAULT_FILE_NAME "hello"

/*
 * Este es el path de nuestro, relativo al punto de montaje, archivo dentro del FS
 */
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

/*
 * Esta es una estructura auxiliar utilizada para almacenar parametros
 * que nosotros le pasemos por linea de comando a la funcion principal
 * de FUSE
*/

struct t_runtime_options {
	char* welcome_msg;
} runtime_options;

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */

#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la metadata de un archivo/directorio. Esto puede ser tamaño, tipo,
 * permisos, dueño, etc ...
 */

static int obtenerAtributos(const char *path, struct stat *stbuf) {

	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));

	/*
		dev_t     st_dev     ID of device containing file
		ino_t     st_ino     file serial number
		mode_t    st_mode    mode of file (see below)
		nlink_t   st_nlink   number of links to the file
		uid_t     st_uid     user ID of file
		gid_t     st_gid     group ID of file
		dev_t     st_rdev    device ID (if file is character or block special)
		off_t     st_size    file size in bytes (if file is a regular file)
		time_t    st_atime   time of last access
		time_t    st_mtime   time of last data modification
		time_t    st_ctime   time of last status change
		blksize_t st_blksize a filesystem-specific preferred I/O block size for this object.  In some filesystem types, this may vary from file to file
		blkcnt_t  st_blocks  number of blocks allocated for this object

		 The following symbolic names for the values of st_mode are also defined:

			File type:

			S_IFMT				S_IFBLK					S_IFCHR
				type of file		block special			character special

			S_IFIFO				S_IFREG					S_IFDIR				S_IFLNK
				FIFO special		regular					directory			symbolic link

			File mode bits:

			S_IRWXU												S_IRUSR							S_IWUSR
				read, write, execute/search by owner				read permission, owner 			write permission, owner

			S_IXUSR										S_IRWXG											S_IRGRP
				execute/search permission, owner			read, write, execute/search by group			read permission, group

			S_IWGRP								S_IXGRP										S_IRWXO
				write permission, group				execute/search permission, group			read, write, execute/search by others

			S_IROTH								S_IWOTH										S_IXOTH
				read permission, others				write permission, others					execute/search permission, others

			S_ISUID								S_ISGID										S_ISVTX
				set-user-ID on execution			set-group-ID on execution					on directories, restricted deletion flag

	 */

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, DEFAULT_FILE_PATH) == 0) {
		stbuf->st_mode = S_IFREG | 0775;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(DEFAULT_FILE_CONTENT); // Aqui toma la metada del archivo HARDCODE
	} else {
		res = -ENOENT;
	}
	return res;
}


/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener la lista de archivos o directorios que se encuentra dentro de un directorio
 */

static int lecturaDiryArch(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, DEFAULT_FILE_NAME, NULL, 0); // Si lo comentamos deja de mostrar el archivo hardcode , debería cambiar la función para que tome los archivos que se generarián

	return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para tratar de abrir un archivo
 */

static int aperturaArch(const char *path, struct fuse_file_info *fi) {

	if (strcmp(path, DEFAULT_FILE_PATH) != 0) // Compara el resultado del open si no lo encuentra da distinto de cero , debería reemplazarse por una función que genere los files
		return -ENOENT;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

/*
 * @DESC
 *  Esta función va a ser llamada cuando a la biblioteca de FUSE le llege un pedido
 * para obtener el contenido de un archivo
 */

// int (*readlink)	   (const char *, char *, size_t);

static int accesoAlContenido(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	size_t len;
	(void) fi;
	if (strcmp(path, DEFAULT_FILE_PATH) != 0) // DEFAULT_FILE_PATH cambiar por una una función que devuelva el archivo creado
		return -ENOENT;

	len = strlen(DEFAULT_FILE_CONTENT);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		 memcpy(buf, DEFAULT_FILE_CONTENT + offset, size); //Esta es la función que nos permite acceder al contenido de un archivo txt por ejemplo si la comentamos mostrará basura
	} else
		size = 0;

	return size;
}

//int(* fuse_operations::create)(const char *, mode_t, struct fuse_file_info *)

/*
static int creacionArchivo(const char * unFile, mode_t, struct fuse_file_info *) {

}
*/

/*
 * Esta es la estructura principal de FUSE con la cual nosotros le decimos a
 * biblioteca que funciones tiene que invocar segun que se le pida a FUSE.
 * Como se observa la estructura contiene punteros a funciones.
 */

static struct fuse_operations hello_oper = {
		.getattr = obtenerAtributos,
		.readdir = lecturaDiryArch,
		.open = aperturaArch,
		.read = accesoAlContenido ,
		/*
		.create = creacionArchivo
	 	.write = escritura,
		.mkdir = crearDirectirio,
		.unlink = eliminarDirectorio,
		.create = crearArchivo,
		.rmdir = eliminarDirectorioVacio
		.flush = funcionRelacionadaConAperturaVi,
		*/
};

/** keys for FUSE_OPT_ options */
enum {
	KEY_VERSION,
	KEY_HELP,
};


/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {
		// Este es un parametro definido por nosotros
		CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};
/*
int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	// Limpio la estructura que va a contener los parametros
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));

	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		// error parsing options
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}

	// Si se paso el parametro --welcome-msg
	// el campo welcome_msg deberia tener el
	// valor pasado
	if( runtime_options.welcome_msg != NULL ){
		printf("%s\n", runtime_options.welcome_msg);
	}

	// Esta es la funcion principal de FUSE, es la que se encarga
	// de realizar el montaje, comuniscarse con el kernel, delegar todo
	// en varios threads
	return fuse_main(args.argc, args.argv, &hello_oper, NULL);
}

*/
