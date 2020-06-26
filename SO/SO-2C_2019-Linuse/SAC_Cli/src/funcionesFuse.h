#ifndef FUNCIONESFUSE_H
#define FUNCIONESFUSE_H

#include "Generales.h"

int socketServer  , conexion ;

#define DEFAULT_FILE_CONTENT "Hello World!\n"

#define DEFAULT_FILE_NAME "hello.txt"

#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

enum {
	KEY_VERSION,
	KEY_HELP,
};

typedef struct  struct_open{
	char* path;
	uint32_t path_tam;
	int create;
	int ensure;
	int trunc;
} GOpen;

typedef struct  struct_path{
	char* path;
	uint32_t path_tam;
} GPath;

typedef struct  struct_pathRename{
	char* path;
	char* pathNew;
	uint32_t path_tam;
	uint32_t pathNew_tam;
} GPathRename;

typedef struct sac_getattr{
	uint8_t state;
	uint32_t file_size;
	unsigned long long updated;
}GAttr;

typedef struct {
	int type;
	char path[PATH_MAX];
} sac_readdir_files;

typedef struct{
	int path_tam;
	char* path;
	uint64_t utime;
}sac_utime_file;


int sac_create( const char *path, mode_t mode, struct fuse_file_info *fi) ;

int sac_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) ;

int sac_open(const char *path, struct fuse_file_info *fi) ;

int sac_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int sac_getattr(const char *path, struct stat *stbuf);

int sac_mkdir( const char *path, mode_t mode);

int sac_mknod( const char *path, mode_t mode, dev_t rdev) ;

int sac_write( const char *path, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);

int sac_unlink( const char *path);

int sac_rmdir( const char *path);

// int sac_flush( const char *path, struct fuse_file_info *fi);

int sac_chmod(const char *path, mode_t mode) ;

int sac_chown(const char *path, uid_t uid, gid_t gid) ;

// void sac_init(struct fuse_conn_info *conn);

int sac_utimes (const char *path, struct timeval tvp[2]);

int sac_rename(const char* path,const char* newName);

int sac_release (const char * path, struct fuse_file_info * fildes) ;

int sac_truncate (const char* path, off_t length);

/*

//Esta estructura es utilizada para decirle a la biblioteca de FUSE que
//parametro puede recibir y donde tiene que guardar el valor de estos

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

 //Esta es una estructura auxiliar utilizada para almacenar parametros
 //que nosotros le pasemos por linea de comando a la funcion principal
 //de FUSE

struct t_runtime_options {
	char* welcome_msg;
} runtime_options;


 //Esta Macro sirve para definir nuestros propios parametros que queremos que
 //FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 //welcome_msg de la variable runtime_options

#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }
*/

static struct fuse_operations sacCli_oper = {

		//.init = sac_init,
		.readdir = sac_readdir,
		.getattr = sac_getattr,
		.open = sac_open,
		.read = sac_read,
		.create = sac_create,
		.mkdir = sac_mkdir,
		.mknod = sac_mknod,
	 	.write = sac_write,
		.utime = sac_utimes,
		.rename = sac_rename,
		.unlink = sac_unlink,
		.rmdir = sac_rmdir,
		//.flush = sac_flush,
		.chown = sac_chown,
		.chmod = sac_chmod ,
		.release = sac_release,
		.truncate = sac_truncate ,
};

#endif
