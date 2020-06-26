#include "libsuse.h"
#include "Generales.h"

int schedule_next() {
	thread* hilo;
	hilo = malloc (sizeof(thread));
	int nextService = 0;
	int threadNotInExecList = TRUE;
	thread* threadInExecute;
	threadInExecute = malloc (sizeof(thread));
	int processInExecute;
	thread* threadToExecute ;
	threadToExecute = malloc (sizeof(thread));
	threadToExecute = list_get(listaReady, 0);
	int estimacionProceso = threadToExecute->estimacion;
	for (int threadPosition = 1; threadPosition < list_size(listaReady); threadPosition++) {
		nextService = hilo->estimacion;
		if (estimacionProceso > nextService) {
			threadToExecute = nextService;
		}
	}
	for (int threadPosition = 0; threadPosition < list_size(listaExec); threadPosition++) {
		threadInExecute = list_get(listaExec, threadPosition);
		processInExecute = threadInExecute->pid;
		if (processInExecute == threadToExecute->pid) {
			list_replace(listaExec, threadPosition, threadToExecute);
			threadNotInExecList = FALSE;
			break;
		}
	}
	if (threadNotInExecList) {
		list_add(listaExec, threadToExecute);
	}
	list_add(listaExec, threadToExecute);
	//timer = obtener_timestamp();
	log_info(logger, "Thread %i en Exec", threadToExecute);
	list_remove(listaReady, threadToExecute);
	threadToExecute->tiempoUsoCPU = obtener_timestamp();
	return threadToExecute->tid;
}

void join(char* threadToJoinStr) {
	int threadToCloseInt = atoi(threadToJoinStr);
	int threadPosition = getIndex(listaExec, threadToCloseInt);
	thread* threadInExec = list_get(listaExec, threadPosition);
	list_add(listaBlocked, threadInExec);
	list_remove(listaExec, threadInExec);
	signal(maxMultiprog);
	log_info(logger, "Thread %i en blocked", threadInExec->tid);
	while(TRUE) {
		if (getIndex(listaExit, threadInExec->tid) != -1) {
			break;
		}
	}
	list_remove(listaBlocked, threadInExec);
	list_add(listaExec, threadInExec);
}



thread* crearTCB(int p_id, int t_id) {
	thread* thread = malloc(sizeof(thread));
	int pid = p_id;
	int tid = t_id++;
	int estimacion = 0;
	int tiempoEspera = 0;
	return thread;
}

unsigned long long obtener_timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long timestamp = ((unsigned long long)tv.tv_sec*1e3) + ((unsigned long long)tv.tv_usec/1000);
	return timestamp;
}

void loguearMetricas() {
	int metricsTimer = configFile->metricsTimer;
	while (true) {
		usleep(metricsTimer);
		metricas->cant_hilos_new = sizeof(listaNew);
		metricas->cant_hilos_ready = sizeof(listaReady);
		metricas->cant_hilos_run = sizeof(listaExec);
		metricas->cant_hilos_blocked = sizeof(listaBlocked);
		metricas->grado_multiprog = multiprog;
	}
}

void mostrar_metricas_consola() {
	/*pthread_mutex_lock(&mutex_metricas);
	unsigned long long read_latency = metricas->tiempo_reads / metricas->cant_reads;
	unsigned long long write_latency = metricas->tiempo_writes / metricas->cant_writes;*/

	printf("*********************METRICAS****************************\n");
	printf("Tiempo de ejecucion = %llu\n",metricas->tiempo_exec);
	printf("Tiempo de espera = %llu\n",metricas->tiempo_waits);
	printf("Tiempo de uso de CPU = %llu\n",metricas->tiempo_cpu);
	printf("Porcentaje de tiempo de ejecución = %i\n",metricas->porc_tiempo_cpu);
	printf("Cantidad de hilos en New = %i\n",metricas->cant_hilos_new);
	printf("Cantidad de hilos en Ready = %i\n",metricas->cant_hilos_ready);
	printf("Cantidad de hilos en Run = %i\n",metricas->cant_hilos_run);
	printf("Cantidad de hilos en Blocked = %i\n",metricas->cant_hilos_blocked);
	printf("Semaforos ID = %i\n",metricas->sem_ID);
	printf("Semaforos Init = %i\n",metricas->sem_Init);
	printf("Semaforos Max = %i\n",metricas->sem_Max);
	printf("Grado de multiprogramacion = %i\n",metricas->grado_multiprog);
	/*int i;
	for(i=0;i<list_size(metricas->memorias_metrics);i++){

		memorias_metrics_t* mem = list_get(metricas->memorias_metrics,i);
		unsigned long long promedio = mem->cant_inserts_y_selects / metricas->cant_inserts_select_total;
		printf("Nro Memoria= %i Memory load = %llu\n",mem->nro_memoria,promedio);
	}

	//Tengo que logear tambien cuando me piden las metricas por consola.
	mostrar_metricas_log();

	pthread_mutex_unlock(&mutex_metricas);*/
}

void mostrar_metricas_log(){



	/*unsigned long long read_latency = metricas->tiempo_reads / metricas->cant_reads;
	unsigned long long write_latency = metricas->tiempo_writes / metricas->cant_writes;

	log_info(logger,"*********************METRICAS****************************\n");
	log_info(logger,"read_latency = %llu\n",read_latency);
	log_info(logger,"write_latency = %llu\n",write_latency);
	int i;
	for(i=0;i<list_size(metricas->memorias_metrics);i++){

		memorias_metrics_t* mem = list_get(metricas->memorias_metrics,i);
		unsigned long long promedio = mem->cant_inserts_y_selects / metricas->cant_inserts_select_total;
		log_info(logger,"Nro Memoria= %i Memory load = %llu\n",mem->nro_memoria,promedio);
	}*/


}

void reset_metricas(){
	metricas->tiempo_exec = 0;
	metricas->tiempo_waits = 0;
	metricas->tiempo_cpu = 0;
	metricas->porc_tiempo_cpu = 0;
	metricas->cant_hilos_new = 0;
	metricas->cant_hilos_ready = 0;
	metricas->cant_hilos_run = 0;
	metricas->cant_hilos_blocked = 0;
	metricas->sem_ID = 0;
	metricas->sem_Init = 0;
	metricas->sem_Max = 0;
	metricas->grado_multiprog = 0;
	list_clean(metricas->memorias_metrics);
}

int create(int pid, int tid) {
	/*int socket = nuevoSocket();
	asociarSocket(socket, configFile->listenPort);
	escucharSocket(socket, configFile->maxMultiprog);*/
	int sizeListaNew = sizeof(listaNew);
	wait(maxMultiprog);
	//int fdDeProceso = aceptarConexionSocket(socket);
	thread* thread = crearTCB(pid, tid);
	list_add(listaNew, thread);
	list_add(listaReady, thread);
	log_info(logger, "Hilo %i del proceso %i en New", tid, pid);
	log_info(logger, "Hilo %i del proceso %i en Ready", tid, pid);
	return 0;
}

int s_close(char* threadToClose) {
	signal(maxMultiprog);
	int threadToCloseInt = atoi(threadToClose);
	int indexListaBlocked;
	int indexListaNew = getIndex(listaNew, threadToCloseInt);
	if (indexListaNew == -1) {
		indexListaBlocked = getIndex(listaBlocked, threadToCloseInt);
		if (indexListaBlocked == -1) {
			log_info(logger, "Thread %i no encontrado", threadToCloseInt);
			return 0;
		} else {
			list_remove(listaBlocked, indexListaBlocked);
			list_add(listaExit, threadToClose);
			log_info(logger, "Proceso %i en Exit", threadToCloseInt);
			return 1;
		}
	} else {
		list_remove(listaNew, indexListaNew);
		list_add(listaExit, threadToClose);
		log_info(logger, "Proceso %i en Exit", threadToCloseInt);
		return 1;
	}
}

int getIndex(t_list* lista, int proceso) {
	int position;
	for(position = 0; list_size(lista) > position; position++) {
		process* element = list_get(lista, position);
		if(element->pid == proceso) {
			return position;
		}
	}
	return -1;
}

/*int irANew(int nuevoProceso) {

	int actualMultiprog = sizeof(listaNew);
	int cantSuspended = sizeof(listaSuspended);
	if (maxMultiprog >= actualMultiprog) {

	} else {
		listaSuspended[cantSuspended + 1] = nuevoProceso;
		return 1;
	}
}*/
