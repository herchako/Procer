/*
 * pruebas.c
 *
 *  Created on: 10/11/2012
 *      Author: Marian-Juli
 */
#include <pthread.h>
#include <commons/listSTS.h>
#include <commons/pcb.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <commons/config.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <signal.h>
#include <commons/log.h>
#include <commons/collections/list.h>

void FUNCTION_LTS();
void FUNCTION_STS();
void FUNCTION_PROCER();
void FUNCTION_IOT(void *pos);
void change_config();
void destruir_pcbs();
void verificar_pendientes();
int16_t load_config();
//int socketServer();
void suspender_proceso(int signal);
int16_t truncar_algoritmo();
void mandar_por_socket(char *arg);
void reanudar_proceso(int16_t pid);
void reanudar_proceso_aux(int signal);
void verificar_pendientes();

t_queueSTS *bloqueados;
t_queueSTS *suspendidos;
t_queueSTS *finIO;
t_queueSTS *finQ;
t_queueSTS *nuevos;
t_queueSTS *reanudados;
t_queueSTS *pendientes;
t_queueSTS *finalizados;
t_pcbListos *ready;
t_pcbListos *logueo_iot;

sem_t element_count_colas;
sem_t elem_bloqueados;
sem_t elem_en_ready;
sem_t sem_para_iniciar;
sem_t iot_disponibles;
sem_t elem_suspendidos;
sem_t elem_finalizados;
sem_t elem_pendientes;

t_queueSTS *prioridad_colas[4];
pthread_mutex_t mutex_prioridad_colas;
pthread_mutex_t mutex_algoritmo;
pthread_mutex_t mutex_tiempo_sentencia;
pthread_mutex_t mutex_mmp_actual;

char *IPHost;
int16_t puertoHost;
int16_t mps;
int16_t mmp;
int16_t mmp_actual;
int16_t mps_actual;
char *algoritmo;
int16_t quantum;
int16_t prioridad_lpn;
int16_t prioridad_lpr;
int16_t prioridad_lpfq;
int16_t prioridad_lpfio;
int16_t tiempo_sentencia;
int16_t tiempo_instruccion_io;
int16_t cantidad_instancias_hilo_iot;
double valor_alfa;
double rafaga_estimada;
double rafaga_real;
char config_path[] =
		"/home/utnso/git/2012-2c-medrano-y-labashe/procer/procerConfigs/pp.cfg";

t_pcb *pcb_ejecutando;
volatile sig_atomic_t no_ejecutar_elem_suspendido = 1;

int main() {

	mmp_actual = 0;
	mps_actual = 0;
	load_config();

	pthread_t LTS;
	pthread_t STS;
	pthread_t PROCER;
	pthread_t IOT[cantidad_instancias_hilo_iot];

	pcb_ejecutando = NULL;

	//Llamada a la función socketServer que permite la conexión con el PI

	//socketServer ();

	sem_init(&element_count_colas, 0, 0);
	sem_init(&elem_bloqueados, 0, 0);
	sem_init(&elem_en_ready, 0, 0);
	sem_init(&sem_para_iniciar, 0, 1); //es sólo para probar para que empieze el LTS
	sem_init(&iot_disponibles, 0, cantidad_instancias_hilo_iot);
	sem_init(&elem_suspendidos, 0, 0);
	sem_init(&elem_finalizados, 0, 0);
	sem_init(&elem_pendientes, 0, 0);

	pthread_mutex_init(&mutex_prioridad_colas, NULL);
	pthread_mutex_init(&mutex_algoritmo, NULL);
	pthread_mutex_init(&mutex_tiempo_sentencia, NULL);
	pthread_mutex_init(&mutex_mmp_actual, NULL);

	bloqueados = queueSTS_create();
	suspendidos = queueSTS_create();
	finIO = queueSTS_create();
	finQ = queueSTS_create();
	nuevos = queueSTS_create();
	reanudados = queueSTS_create();
	pendientes = queueSTS_create();
	ready = pcbListos_create();
	finalizados = queueSTS_create();
	logueo_iot = pcbListos_create();

	prioridad_colas[prioridad_lpn - 1] = nuevos;
	prioridad_colas[prioridad_lpr - 1] = reanudados;
	prioridad_colas[prioridad_lpfio - 1] = finIO;
	prioridad_colas[prioridad_lpfq - 1] = finQ;

	if (pthread_create(&LTS, NULL, (void*) &FUNCTION_LTS, NULL)) {
		perror("Error en la creación del lts.");
		return EXIT_FAILURE;
	}
	if (pthread_create(&STS, NULL, (void*) &FUNCTION_STS, NULL)) {
		perror("Error en la creación del sts.");
		return EXIT_FAILURE;
	}
	if (pthread_create(&PROCER, NULL, (void*) &FUNCTION_PROCER, NULL)) {
		perror("Error en la creación de procer.");
		return EXIT_FAILURE;
	}

	int i;
	int *args = malloc(sizeof(int) * cantidad_instancias_hilo_iot);
	for (i = 0; i < cantidad_instancias_hilo_iot; i++) {
		args[i] = i;
		if (pthread_create(&IOT[i], NULL, (void*) &FUNCTION_IOT, &(args[i]))) {
			perror("Error en la creación de thread iot.");
			return EXIT_FAILURE;
		}
	}

	int j;
	for (j = 0; j < cantidad_instancias_hilo_iot; j++) {
		int *aux = malloc(sizeof(int));
		*aux = 1;
		list_add(logueo_iot->listos, aux);
	}

	pthread_join(LTS, NULL);
	pthread_join(STS, NULL);
	pthread_join(PROCER, NULL);

	int a;
	for (a = 0; a < cantidad_instancias_hilo_iot; a++) {
		pthread_join(IOT[i], NULL);

	}
	free(args);

	queueBloqueados_destroy(bloqueados);
	queueSTS_destroy(suspendidos);
	queueSTS_destroy(finIO);
	queueSTS_destroy(finQ);
	queueSTS_destroy(nuevos);
	queueSTS_destroy(reanudados);
	queueSTS_destroy(pendientes);
	pcbListos_destroy(ready);

	sem_destroy(&element_count_colas);
	sem_destroy(&elem_en_ready);
	sem_destroy(&elem_bloqueados);
	sem_destroy(&sem_para_iniciar);
	sem_destroy(&iot_disponibles);
	sem_destroy(&elem_suspendidos);
	sem_destroy(&elem_finalizados);

	pthread_mutex_destroy(&mutex_algoritmo);
	pthread_mutex_destroy(&mutex_prioridad_colas);
	pthread_mutex_destroy(&mutex_tiempo_sentencia);
	free(algoritmo);
	free(IPHost);
	return 0;
}

void FUNCTION_LTS() {
	sem_wait(&sem_para_iniciar);

	printf(
			"Para suspender el proceso en ejecución ingresar kill -SIGUSR1 %d \n",
			getpid());
	signal(SIGUSR1, suspender_proceso);
	printf("Para reanudar un proceso ingresar kill -SIGUSR2 %d \n", getpid());
	signal(SIGUSR2, reanudar_proceso_aux);

	t_pcb *pcb1 = malloc(sizeof(t_pcb));
	cargar_pcb("/home/utnso/archivo", pcb1);
//	mostrar_pcb(pcb1);
	mmp_actual++;

	t_pcb *pcb2 = malloc(sizeof(t_pcb));
	cargar_pcb("/home/utnso/script Ansisop", pcb2);
//	mostrar_pcb(pcb2);
	mmp_actual++;

	push_queueSTS(nuevos, pcb1);
	function_logueo_lsch(pcb1->pid, 0, "Nuevos", "LTS", -1, 0, 0, 0, 0);
	sem_post(&element_count_colas);

	push_queueSTS(nuevos, pcb2);
	function_logueo_lsch(pcb2->pid, 0, "Nuevos", "LTS", -1, 0, 0, 0, 0);
	sem_post(&element_count_colas);

	pthread_t procesos_finalizados;
	if (pthread_create(&procesos_finalizados, NULL, (void*) &destruir_pcbs,
			NULL)) {
		perror("Error en la creación de procesos finalizados");
	}



	while (1) {
//		sleep(5);
	}

}

void FUNCTION_STS() {
	pthread_t inotify;
	if (pthread_create(&inotify, NULL, (void*) &change_config, NULL)) {
		perror("Error en la creación de inotify.");
	}
	while (1) {
		sem_wait(&element_count_colas);
		int i;
		for (i = 0; i < 4; i++) {
			pthread_mutex_lock(&mutex_prioridad_colas);
			pthread_mutex_lock(&prioridad_colas[i]->mutex);
			t_pcb *pcb;
			if (prioridad_colas[i]->queue->elements->elements_count != 0) {
				while (prioridad_colas[i]->queue->elements->elements_count != 0) {
					pcb = queue_pop(prioridad_colas[i]->queue);
					//pthread_mutex_lock(&mutex_algoritmo);
					int num_algoritmo = truncar_algoritmo();
					switch (num_algoritmo) {
					case 1: // RR
						insertar_ordenado_FIFO(ready, pcb);
						function_logueo_lsch(pcb->pid, 0, "Ready", "STS", i,
								prioridad_lpn, prioridad_lpr, prioridad_lpfq,
								prioridad_lpfio);
						sem_post(&elem_en_ready);
						sleep(1);
						break;
					case 2: //SPN
						insertar_ordenado_SPN(ready, pcb);
						function_logueo_lsch(pcb->pid, 0, "Ready", "STS", i,
								prioridad_lpn, prioridad_lpr, prioridad_lpfq,
								prioridad_lpfio);
						sleep(1);
						sem_post(&elem_en_ready);
						break;
					case 3: //FIFO
						insertar_ordenado_FIFO(ready, pcb);
						function_logueo_lsch(pcb->pid, 0, "Ready", "STS", i,
								prioridad_lpn, prioridad_lpr, prioridad_lpfq,
								prioridad_lpfio);
						sleep(1);
						sem_post(&elem_en_ready);
						break;
					case 4: //PRI
						insertar_ordenado_PRI(ready, pcb);
						function_logueo_lsch(pcb->pid, 0, "Ready", "STS", i,
								prioridad_lpn, prioridad_lpr, prioridad_lpfq,
								prioridad_lpfio);
						sem_post(&elem_en_ready);
						sleep(1);
						break;
					}
					//pthread_mutex_unlock(&mutex_algoritmo);
				}
			}
			pthread_mutex_unlock(&prioridad_colas[i]->mutex);
			pthread_mutex_unlock(&mutex_prioridad_colas);
		}
	}

}

void FUNCTION_PROCER() {
	int32_t tiempo = 0;
	int resultado = -1;
	int value;
	int continuar_ejecutando = 1;
	char *variableIO = malloc(2);
	int tiempo_aux;
	int valor;
	while (1) {

		if (pcb_ejecutando == NULL || truncar_algoritmo() == 1) {
			sem_wait(&elem_en_ready);
			pthread_mutex_lock(&ready->mutex);
			no_ejecutar_elem_suspendido = 1;
			continuar_ejecutando = 1;
			pcb_ejecutando = (t_pcb *) list_remove(ready->listos, 0);
			pthread_mutex_unlock(&ready->mutex);
			printf("Ejecutando pcb %d\n", pcb_ejecutando->pid);
			int i = 0;

			while (i < quantum && no_ejecutar_elem_suspendido
					&& continuar_ejecutando) {
				printf("Procer ejecutando instruccion %d\n", i);
				strcpy(variableIO, "0");
				resultado = ejecutar(pcb_ejecutando, &tiempo, &variableIO);
				printf(
						"Procer termino de ejecutar instruccion %d de tiempo %d\n",
						i, tiempo);
				i++;
				switch (resultado) { //Evaluo lo que me devuelve ejecutar_pcb
				case 0: //termino de ejecutar
					puts("termino de ejecutar");
					continuar_ejecutando = 0;
					push_queueSTS(finalizados, pcb_ejecutando);
					sem_post(&elem_finalizados);
					char sms[40];
					sprintf(sms, "Ha finalizado el proceso %d",
							pcb_ejecutando->pid);
					function_logueo("PP", "PROCER", "INFO", sms);
					function_logueo_lsch(pcb_ejecutando->pid, "Ready",
							"Finalizados", "PROCER", -1, 0, 0, 0, 0);
					pcb_ejecutando = NULL;
					break;
				case 1: // i/o bloqueante
					puts("io bloqueante");
					pthread_mutex_lock(&mutex_tiempo_sentencia);
					tiempo_aux = tiempo_sentencia;
					pthread_mutex_unlock(&mutex_tiempo_sentencia);
					sleep(tiempo_aux);
					if (strcmp("0", variableIO) != 0)
					reemplazar_valor_seg_datos(pcb_ejecutando,
							variableIO, 0);

					push_queueBloqueados(bloqueados, pcb_ejecutando,
							tiempo * tiempo_instruccion_io);
					sem_post(&elem_bloqueados);
					function_logueo_lsch(pcb_ejecutando->pid, "Ready",
							"Bloqueados", "PROCER", -1, 0, 0, 0, 0);
					pcb_ejecutando = NULL;
					continuar_ejecutando = 0;
					break;
				case 2: // i/o no bloqueante
					puts("no bloqueante");
					pthread_mutex_lock(&mutex_tiempo_sentencia);
					tiempo_aux = tiempo_sentencia;
					pthread_mutex_unlock(&mutex_tiempo_sentencia);
					sleep(tiempo_aux);
					sem_getvalue(&iot_disponibles, &value);
					if (value <= 0) { //no hay hilos disponibles
						continuar_ejecutando = 1;
						if (strcmp("0", variableIO) != 0)
							reemplazar_valor_seg_datos(pcb_ejecutando, variableIO, 1);

						char msj[50];
						sprintf(msj,
								"El proceso %d quiso ejecutar una IO no bloqueante, pero no había dispostivos disponibles",
								pcb_ejecutando->pid);
						function_logueo("PP", "PROCER", "WARN", msj);
						int pos;
						pthread_mutex_lock(&logueo_iot->mutex);
						for (pos = 0; pos < cantidad_instancias_hilo_iot;
								pos++) {
							int *pop;
							pop = list_get(logueo_iot->listos, pos);
							*pop = *pop + 1;
						}
						pthread_mutex_unlock(&logueo_iot->mutex);
						puts("no pude ejecutar");
					} else {
						if (strcmp("0", variableIO) != 0)
							reemplazar_valor_seg_datos(pcb_ejecutando, variableIO, 0);
						push_queueBloqueados(bloqueados, pcb_ejecutando,
								tiempo * tiempo_instruccion_io);
						sem_post(&elem_bloqueados);
						function_logueo_lsch(pcb_ejecutando->pid, "Ready",
								"Bloqueados", "PROCER", -1, 0, 0, 0, 0);
						pcb_ejecutando = NULL;
						continuar_ejecutando = 0;
						puts("pude ejecutar");
					}
					break;
				case 3: //Ejecuto instruccion normalmente. No fue IO ni fin de programa
					puts("Ni IO ni fin programa");
					if (tiempo == 0) {
						pthread_mutex_lock(&mutex_tiempo_sentencia);
						tiempo_aux = tiempo_sentencia;
						pthread_mutex_unlock(&mutex_tiempo_sentencia);
						sleep(tiempo_aux);
					} else {
						sleep(tiempo);
					}
					continuar_ejecutando = 1;
					break;
				}

			}
			if (truncar_algoritmo() == 1 && pcb_ejecutando != NULL && no_ejecutar_elem_suspendido) {
				push_queueSTS(finQ, pcb_ejecutando);
				sem_post(&element_count_colas);
				function_logueo_lsch(pcb_ejecutando->pid, "Ready", "FinQ",
						"PROCER", -1, 0, 0, 0, 0);
				continuar_ejecutando = 0;
			}

		} else {
			continuar_ejecutando = 1;
			no_ejecutar_elem_suspendido = 1;
			printf("Ejecutando pcb %d\n", pcb_ejecutando->pid);
			int i = 0;
			while (i < quantum && no_ejecutar_elem_suspendido
					&& continuar_ejecutando) {
				printf("Procer ejecutando instruccion %d\n", i);
				strcpy(variableIO, "0");
				resultado = ejecutar(pcb_ejecutando, &tiempo, &variableIO);
				printf(
						"Procer termino de ejecutar instruccion %d de tiempo %d\n",
						i, tiempo);
				i++;
				switch (resultado) { //Evaluo lo que me devuelve ejecutar_pcb
				case 0: //termino de ejecutar
					puts("termino de ejecutar");
					continuar_ejecutando = 0;
					push_queueSTS(finalizados, pcb_ejecutando);
					sem_post(&elem_finalizados);
					char sms[40];
					sprintf(sms, "Ha finalizado el proceso %d",
							pcb_ejecutando->pid);
					function_logueo("PP", "PROCER", "INFO", sms);
					function_logueo_lsch(pcb_ejecutando->pid, "Ready",
							"Finalizados", "PROCER", -1, 0, 0, 0, 0);
					pcb_ejecutando = NULL;
					break;
				case 1: // i/o bloqueante
					puts("io bloqueante");
					pthread_mutex_lock(&mutex_tiempo_sentencia);
					tiempo_aux = tiempo_sentencia;
					pthread_mutex_unlock(&mutex_tiempo_sentencia);
					sleep(tiempo_aux);
					if (strcmp("0", variableIO) != 0)
						reemplazar_valor_seg_datos(pcb_ejecutando, variableIO, 0);
					push_queueBloqueados(bloqueados, pcb_ejecutando,
							tiempo * tiempo_instruccion_io);
					sem_post(&elem_bloqueados);
					function_logueo_lsch(pcb_ejecutando->pid, "Ready",
							"Bloqueados", "PROCER", -1, 0, 0, 0, 0);
					pcb_ejecutando = NULL;
					continuar_ejecutando = 0;
					break;
				case 2: // i/o no bloqueante
					puts("no bloqueante");
					pthread_mutex_lock(&mutex_tiempo_sentencia);
					tiempo_aux = tiempo_sentencia;
					pthread_mutex_unlock(&mutex_tiempo_sentencia);
					sleep(tiempo_aux);
					sem_getvalue(&iot_disponibles, &value);
					if (value <= 0) { //no hay hilos disponibles
						continuar_ejecutando = 1;
						if (strcmp("0", variableIO) != 0)
							reemplazar_valor_seg_datos(pcb_ejecutando, variableIO, 1);
						char msj[50];
						sprintf(msj,
								"El proceso %d quiso ejecutar una IO no bloqueante, pero no había dispostivos disponibles",
								pcb_ejecutando->pid);
						function_logueo("PP", "PROCER", "WARN", msj);
						int pos;
						pthread_mutex_lock(&logueo_iot->mutex);
						for (pos = 0; pos < cantidad_instancias_hilo_iot;
								pos++) {
							int *pop;
							pop = list_get(logueo_iot->listos, pos);
							*pop = *pop + 1;
						}
						pthread_mutex_unlock(&logueo_iot->mutex);
						puts("no pude ejecutar");

					} else {
						if (strcmp("0", variableIO) != 0)
							reemplazar_valor_seg_datos(pcb_ejecutando, variableIO, 0);
						push_queueBloqueados(bloqueados, pcb_ejecutando,
								tiempo * tiempo_instruccion_io);
						sem_post(&elem_bloqueados);
						function_logueo_lsch(pcb_ejecutando->pid, "Ready",
								"Bloqueados", "PROCER", -1, 0, 0, 0, 0);
						continuar_ejecutando = 0;
						pcb_ejecutando = NULL;
						puts(
								"io no bloqueante, hilos disponibles, voy a bloqueados");
					}
					break;
				case 3:
					if (tiempo == 0) { //Evalua si se cambio el tiempo de instrucción
						pthread_mutex_lock(&mutex_tiempo_sentencia);
						tiempo_aux = tiempo_sentencia;
						pthread_mutex_unlock(&mutex_tiempo_sentencia);
						sleep(tiempo_aux);
					} else {
						sleep(tiempo);
					}
					break;
				}

			}

		}
		if (no_ejecutar_elem_suspendido == 0 || continuar_ejecutando == 0) {
			pcb_ejecutando = NULL;
		}
	}
}

void FUNCTION_IOT(void *pos) {
	int *posicion = (int*) pos;
	while (1) {
		sem_wait(&elem_bloqueados);
		sem_wait(&iot_disponibles);
		t_io *elem_pop;
		elem_pop = pop_queueBloqueados(bloqueados);
		sleep(elem_pop->tiempo_io);
		push_queueSTS(finIO, elem_pop->pcb);
		sem_post(&element_count_colas);
		function_logueo_lsch(elem_pop->pcb->pid, "Bloqueados", "FinIO", "IOT",
				-1, 0, 0, 0, 0);
		pthread_mutex_lock(&logueo_iot->mutex);
		int *cant_logueo;
		cant_logueo = list_get(logueo_iot->listos, *posicion);
		if (*cant_logueo != 0) {
			int aux;
			for (aux = 0; aux < *cant_logueo; aux++) {
				char msj[50];
				sprintf(msj,
						"Se quiso realizar una IO no bloqueante, y el dispositivo estaba siendo usado por el proceso %d",
						elem_pop->pcb->pid);
				function_logueo("PP", "IOT", "WARN", msj);
			}
			*cant_logueo = 0;
		}

		pthread_mutex_unlock(&logueo_iot->mutex);
		free(elem_pop);
		sem_post(&iot_disponibles);

	}

}

void change_config() {
	int32_t EVENT_SIZE = (sizeof(struct inotify_event) + 24);
	int32_t BUF_LEN = (1024 * EVENT_SIZE);
	while (1) {
		char buffer[BUF_LEN];

		int file_descriptor = inotify_init();
		if (file_descriptor < 0) {
			perror("inotify_init");
		}

		int watch_descriptor = inotify_add_watch(file_descriptor, config_path,
				IN_CLOSE_WRITE);

		int length = read(file_descriptor, buffer, BUF_LEN);
		if (length < 0) {
			perror("read");
		}

		int offset = 0;

		while (offset < length) {

			struct inotify_event *event =
					(struct inotify_event *) &buffer[offset];
			if (event->mask & IN_CLOSE_WRITE) {
				function_logueo("PP", 0, "INFO",
						"Hubo una modificación en el archivo de configuración.");
				pthread_mutex_lock(&mutex_prioridad_colas);
				pthread_mutex_lock(&mutex_tiempo_sentencia);
				load_config();
				int num_algoritmo = truncar_algoritmo();
				switch (num_algoritmo) {
				case 1: // RR
					reordenar_a_FIFO(ready);
					break;
				case 2: //SPN
					reordenar_a_SPN(ready);
					break;
				case 3: //FIFO
					reordenar_a_FIFO(ready);
					break;
				case 4: //PRI
					reordenar_a_PRI(ready);
					break;
				}
				prioridad_colas[prioridad_lpn - 1] = nuevos;
				prioridad_colas[prioridad_lpr - 1] = reanudados;
				prioridad_colas[prioridad_lpfio - 1] = finIO;
				prioridad_colas[prioridad_lpfq - 1] = finQ;
				pthread_mutex_unlock(&mutex_tiempo_sentencia);
				pthread_mutex_unlock(&mutex_prioridad_colas);
			}
			offset += sizeof(struct inotify_event) + event->len;
		}

		inotify_rm_watch(file_descriptor, watch_descriptor);
		close(file_descriptor);
	}
}

int16_t load_config() {
	char *stringAux;
	char *stringAux2;
	t_config *arch_config_pp = config_create(config_path);
	stringAux = config_get_string_value(arch_config_pp, "pp.host");
	IPHost = malloc(strlen(stringAux) + 1);
	strcpy(IPHost, stringAux);

	puertoHost = config_get_int_value(arch_config_pp, "pp.puerto");
	mps = config_get_int_value(arch_config_pp, "pp.mps");
	mmp = config_get_int_value(arch_config_pp, "pp.mmp");
	pthread_mutex_lock(&mutex_algoritmo);
	stringAux2 = config_get_string_value(arch_config_pp, "pp.algoritmo");
	algoritmo = malloc(strlen(stringAux2) + 1);
	strcpy(algoritmo, stringAux2);
	pthread_mutex_unlock(&mutex_algoritmo);
	quantum = config_get_int_value(arch_config_pp, "pp.quantum");
	prioridad_lpn = config_get_int_value(arch_config_pp, "pp.prioridad_lpn");
	prioridad_lpr = config_get_int_value(arch_config_pp, "pp.prioridad_lpr");
	prioridad_lpfq = config_get_int_value(arch_config_pp, "pp.prioridad_lpfq");
	prioridad_lpfio = config_get_int_value(arch_config_pp,
			"pp.prioridad_lpfio");
	tiempo_sentencia = config_get_int_value(arch_config_pp,
			"pp.tiempo_sentencia");
	tiempo_instruccion_io = config_get_int_value(arch_config_pp,
			"pp.tiempo_instruccion_io");
	cantidad_instancias_hilo_iot = config_get_int_value(arch_config_pp,
			"pp.cantidad_instancias_hilo_iot");
	valor_alfa = config_get_double_value(arch_config_pp, "pp.alfa");
	rafaga_real = config_get_double_value(arch_config_pp, "pp.rafaga_real");
	rafaga_estimada = config_get_double_value(arch_config_pp,
			"pp.rafaga_estimada");

	function_logueo("PP", 0, "INFO",
			"Se ha cargado el archivo de configuración");

	printf("---VARIABLES CARGADAS DESDE CONFIG---\n");
	printf("Ip Host: %s\n", IPHost);
	printf("Puerto Host: %d\n", puertoHost);
	printf("MPS: %d\n", mps);
	printf("MMP: %d\n", mmp);
	printf("(ALGORITMO ORDENAMIENTO)LPL: %s\n", algoritmo);
	printf("Quantum para algoritmos expropiativos: %d\n", quantum);
	printf("LPN: %d\n", prioridad_lpn);
	printf("LPR: %d\n", prioridad_lpr);
	printf("LPFQ: %d\n", prioridad_lpfq);
	printf("LPFIO: %d\n", prioridad_lpfio);
	printf("TIEMPO SENTENCIA: %d\n", tiempo_sentencia);
	printf("TIEMPO INSTRUCCION IO: %d\n", tiempo_instruccion_io);
	printf("CANTIDAD HILOS IOT: %d\n", cantidad_instancias_hilo_iot);
	printf("VALOR DE ALFA: %f\n", valor_alfa);
	config_destroy(arch_config_pp);

	return EXIT_SUCCESS;
}

int16_t truncar_algoritmo() {
	int a = 0;
	pthread_mutex_lock(&mutex_algoritmo);
	if (strcmp(algoritmo, "RR") == 0)
		a = 1;
	else if (strcmp(algoritmo, "SPN") == 0)
		a = 2;
	else if (strcmp(algoritmo, "FIFO") == 0)
		a = 3;
	else if (strcmp(algoritmo, "PRI") == 0)
		a = 4;
	pthread_mutex_unlock(&mutex_algoritmo);
	return a;
}

void suspender_proceso(int signal) {
	if (pcb_ejecutando != NULL) {
		char msj[40];
		sprintf(msj, "Se ha suspendido el proceso %d", pcb_ejecutando->pid);
		function_logueo("PP", "LTS", "DEBUG", msj);
		printf("PID pcb suspendido: %d\n", pcb_ejecutando->pid);
		char *estado;
//		estado = guardar_estado_pcb(pcb_ejecutando);
//		mandar_por_socket(estado);
		push_queueSTS(suspendidos, pcb_ejecutando);
		sem_post(&elem_suspendidos);
		function_logueo_lsch(pcb_ejecutando->pid, 0, "Suspendidos", 0, -1, 0, 0,
				0, 0);
		mmp_actual--;
		no_ejecutar_elem_suspendido = 0;
		verificar_pendientes();
	} else {
		puts(
				"Se intentó suspender un proceso, pero no había niguno en ejecución");
		function_logueo("PP", "LTS", "WARN",
				"Se intentó suspender un proceso, pero no había ninguno en ejecución");
	}

}


void destruir_pcbs() {
	while (1) {
		sem_wait(&elem_finalizados);
		t_pcb *pcb;
		pthread_mutex_lock(&finalizados->mutex);
		pcb = queue_pop(finalizados->queue);
		mostrar_pcb(pcb);
		pthread_mutex_unlock(&finalizados->mutex);
		char msj[40];
		sprintf(msj, "Se destruyó el proceso %d", pcb->pid);
		function_logueo("PP", "LTS", "INFO", msj);
		destruir_pcb(pcb);
		puts("pcb destruido");
		mandar_por_socket("Termino el programa");
		pthread_mutex_lock(&mutex_mmp_actual);
		mmp_actual--;
		pthread_mutex_unlock(&mutex_mmp_actual);
		verificar_pendientes();
	}

}

void mandar_por_socket(char *arg) {
	printf("Recibi: %s\n", arg);
}

void reanudar_proceso_aux(int signal) {
	t_pcb *aux = queue_peek(suspendidos->queue);
	reanudar_proceso(aux->pid);
	printf("Elementos de suspendidos: %d\n",
			suspendidos->queue->elements->elements_count);
	printf("Elementos de reanudados: %d\n",
			reanudados->queue->elements->elements_count);
	printf("Elementos de pendientes: %d\n",
			pendientes->queue->elements->elements_count);
}

void reanudar_proceso(int16_t pid) {
	sem_wait(&elem_suspendidos);
	pthread_mutex_lock(&suspendidos->mutex);
	int pid_es_igual(t_pcb *p) {
		return pid == p->pid;
	}
	t_pcb *pcbAux = list_remove_by_condition(suspendidos->queue->elements,
			(void*) pid_es_igual);
	pthread_mutex_unlock(&suspendidos->mutex);
	pthread_mutex_lock(&mutex_mmp_actual);
	if (mmp_actual < mmp) {
		mmp_actual++;
		pthread_mutex_unlock(&mutex_mmp_actual);
		push_queueSTS(reanudados, pcbAux);
		sem_post(&element_count_colas);
		char msj[40];
		sprintf(msj, "Se reaunudó el proceso %d", pcbAux->pid);
		function_logueo("PP", "LTS", "INFO", msj);
		function_logueo_lsch(pcbAux->pid, "Suspendidos", "Reanudados", 0, -1, 0,
				0, 0, 0);
	} else {
		pthread_mutex_unlock(&mutex_mmp_actual);
		push_queueSTS(pendientes, pcbAux);
		sem_post(&elem_pendientes);
		function_logueo_lsch(pcbAux->pid, "Suspendidos", "Pendientes", 0, -1, 0,
				0, 0, 0);
	}
}

void verificar_pendientes() {
	int cant_elementos;
	sem_getvalue(&elem_pendientes, &cant_elementos);
	if (cant_elementos != 0) {
		sem_wait(&elem_pendientes);
		pthread_mutex_lock(&pendientes->mutex);
		t_pcb *pcbAux;
		pcbAux = queue_pop(pendientes->queue);
		pthread_mutex_unlock(&pendientes->mutex);
		push_queueSTS(nuevos, pcbAux);
		sem_post(&element_count_colas);
		pthread_mutex_lock(&mutex_mmp_actual);
		mmp_actual++;
		pthread_mutex_unlock(&mutex_mmp_actual);
		function_logueo_lsch(pcbAux->pid, "Pendientes", "Nuevos", "LTS", -1, 0,
				0, 0, 0);
	}
}
