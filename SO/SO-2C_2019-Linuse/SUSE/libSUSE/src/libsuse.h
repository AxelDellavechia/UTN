#ifndef LIBSUSE_H_
#define LIBSUSE_H_

#define FALSE 0
#define TRUE 1
#define ERROR -1

#include "Sockets.h"
#include "Generales.h"

#include <commons/log.h>
#include <commons/config.h>
#include <semaphore.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <sys/inotify.h>

typedef struct{
	unsigned long long tiempo_exec;
	unsigned long long tiempo_waits;
	unsigned long long tiempo_cpu;
	int porc_tiempo_cpu;
	int cant_hilos_new;
	int cant_hilos_ready;
	int cant_hilos_run;
	int cant_hilos_blocked;
	int sem_ID;
	int sem_Init;
	int sem_Max;
	int grado_multiprog;
	t_list*  memorias_metrics;

} metricas_t;

typedef struct {
	int pid;
	int tid;
	int estimacion;
	int tiempoEspera;
	int tiempoUsoCPU;
	int tiempoExec;
}thread;

t_log* logger;

metricas_t* metricas;

t_list* listaBlocked;
t_list* listaNew;
t_list* listaReady;
t_list* listaExec;
t_list* listaSuspended;
t_list* listaExit;

int multiprog;

unsigned long long timer;

//FUNCIONES

int schedule_next();
thread* crearTCB(int p_id, int t_id);
unsigned long long obtener_timestamp();
void loguearMetricas();
void mostrar_metricas_consola();
void mostrar_metricas_log();
void reset_metricas();
int create(int pid, int tid);
int s_close(char* threadToClose);
int getIndex(t_list* lista, int proceso);
void join(char* threadToJoinStr);


#endif
