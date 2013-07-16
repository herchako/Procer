/*
 * listSTS.c
 *
 *  Created on: 03/11/2012
 *      Author: utnso
 */

#include "listSTS.h"
#include <stdint.h>
#include "collections/queue.h"
#include "pcb.h"
#include <pthread.h>
#include <sys/timeb.h>
#include <string.h>


static int64_t obtener_instante_llegada() {
	struct timeb t;
	ftime(&t);
	int64_t tiempo = (int64_t)(t.time)*1000 + (int64_t)t.millitm;
	return tiempo;
}

static int8_t destruir_io (t_io *io){
	destruir_pcb(io->pcb);
	free(io);
	return EXIT_SUCCESS;
}

t_queueSTS *queueSTS_create() {
	t_queueSTS *queue = malloc(sizeof(t_queueSTS));
	pthread_mutex_init(&queue->mutex, NULL);
	queue->queue = queue_create();
	return queue;
}

t_pcbListos *pcbListos_create() {
	t_pcbListos *listos = malloc(sizeof(t_pcbListos));
	pthread_mutex_init(&listos->mutex, NULL);
	listos->listos = list_create();
	return listos;
}
void pcbListos_destroy(t_pcbListos *listos) {
	list_destroy_and_destroy_elements(listos->listos, (void*) destruir_pcb);
	pthread_mutex_destroy(&listos->mutex);

}

void queueSTS_destroy(t_queueSTS *queue) {
	queue_destroy_and_destroy_elements(queue->queue, (void*) destruir_pcb);
	pthread_mutex_destroy(&queue->mutex);

}

void queueBloqueados_destroy(t_queueSTS *queue) {
	queue_destroy_and_destroy_elements(queue->queue, (void*) destruir_io);
	pthread_mutex_destroy(&queue->mutex);

}



t_io *pop_queueBloqueados(t_queueSTS *queue) {
	pthread_mutex_lock(&queue->mutex);
	t_io *io;
	io = queue_pop(queue->queue);
	pthread_mutex_unlock(&queue->mutex);
	return io;
}

void push_queueSTS (t_queueSTS* queue, t_pcb* pcb){
	pthread_mutex_lock(&queue->mutex);
	queue_push(queue->queue, pcb);
	pthread_mutex_unlock(&queue->mutex);
}

void push_queueBloqueados(t_queueSTS* queue, t_pcb* pcb, unsigned int tiempo) {
	t_io *io = malloc(sizeof(t_io));
	io->pcb = pcb;
	io->tiempo_io = tiempo;
	pthread_mutex_lock(&queue->mutex);
	queue_push(queue->queue, io);
	pthread_mutex_unlock(&queue->mutex);
}

void insertar_ordenado_FIFO(t_pcbListos* lista, t_pcb *pcb) {
	pthread_mutex_lock(&lista->mutex);
	pcb->raf_actual = ((pcb->raf_actual * valor_alfa) + (pcb->ult_rafaga * (1 - valor_alfa)));
	pcb->instante_llegada = obtener_instante_llegada();
	list_add(lista->listos, pcb);
	pthread_mutex_unlock(&lista->mutex);
}

void insertar_ordenado_PRI(t_pcbListos* lista, t_pcb *pcb) {
	pthread_mutex_lock(&lista->mutex);
	int i;
	t_pcb *pcbAux;
	pcb->raf_actual = ((pcb->raf_actual * valor_alfa) + (pcb->ult_rafaga * (1 - valor_alfa)));
	if (list_is_empty(lista->listos)) {
		pcb->instante_llegada = obtener_instante_llegada();
		list_add(lista->listos, pcb);
		pthread_mutex_unlock(&lista->mutex);
	} else {
		for (i = 0; i < list_size(lista->listos); i++) {
			pcbAux = (t_pcb *) list_get(lista->listos, i);
			if (pcbAux->prioridad >= pcb->prioridad)
				break;
		}if(lista->listos->elements_count==i) {
			pcb->instante_llegada = obtener_instante_llegada();
			list_add(lista->listos, pcb);
		}
		else{
			pcb->instante_llegada = obtener_instante_llegada();
			list_add_in_index(lista->listos, i, pcb);
			}
		pthread_mutex_unlock(&lista->mutex);
	}

}

void insertar_ordenado_SPN(t_pcbListos* lista, t_pcb *pcb) {
	pthread_mutex_lock(&lista->mutex);
	int i;
	t_pcb *pcbAux;
	pcb->raf_actual = ((pcb->raf_actual * valor_alfa) + (pcb->ult_rafaga * (1 - valor_alfa)));
	if (list_is_empty(lista->listos)) {
		pcb->instante_llegada = obtener_instante_llegada();
		list_add(lista->listos, pcb);
		pthread_mutex_unlock(&lista->mutex);
	} else {
		for (i = 0; i < list_size(lista->listos); i++) {
			pcbAux = (t_pcb *) list_get(lista->listos, i);
			if (pcbAux->raf_actual >= pcb->raf_actual)
				break;
		}if(lista->listos->elements_count==i) {
			pcb->instante_llegada = obtener_instante_llegada();
			list_add(lista->listos, pcb);
		}
		else{
			pcb->instante_llegada = obtener_instante_llegada();
			list_add_in_index(lista->listos, i, pcb);
			}
		pthread_mutex_unlock(&lista->mutex);
	}

}

void reordenar_a_PRI (t_pcbListos* lista){
	pthread_mutex_lock(&lista->mutex);
	int i;
	int a;
	t_pcb *pcbAux;
	t_pcb *pcb;
	t_list *listaAux;
	listaAux = list_take_and_remove(lista->listos, lista->listos->elements_count);
	for (i=0; i < listaAux->elements_count; i++) {

		pcbAux = (t_pcb*) list_get(listaAux,i);
		if(list_is_empty(lista->listos))
			list_add(lista->listos, pcbAux);
		else {
			for (a=0; a<lista->listos->elements_count; a++){
				pcb = (t_pcb*)list_get(lista->listos,a);
				if(pcb->prioridad >=pcbAux->prioridad)
					break;
				}
			if (lista->listos->elements_count==a){
				list_add(lista->listos,pcbAux);
			}else{
				list_add_in_index(lista->listos,a,pcbAux);
			}
		}

	}
	list_destroy(listaAux);
	pthread_mutex_unlock(&lista->mutex);
}

void reordenar_a_SPN (t_pcbListos* lista){
	pthread_mutex_lock(&lista->mutex);
	int i;
	int a;
	t_pcb *pcbAux;
	t_pcb *pcb;
	t_list *listaAux;
	listaAux = list_take_and_remove(lista->listos, lista->listos->elements_count);
	for (i=0; i < listaAux->elements_count; i++) {
		pcbAux = (t_pcb*) list_get(listaAux,i);
		if(list_is_empty(lista->listos))
			list_add(lista->listos, pcbAux);
		else {
			for (a=0; a<lista->listos->elements_count; a++){
				pcb = (t_pcb*)list_get(lista->listos,a);
				if(pcb->raf_actual >=pcbAux->raf_actual)
					break;
				}
			if (lista->listos->elements_count==a){
				list_add(lista->listos,pcbAux);
			}else{
				list_add_in_index(lista->listos,a,pcbAux);
			}
		}

	}
	list_destroy(listaAux);
	pthread_mutex_unlock(&lista->mutex);
}

void reordenar_a_FIFO (t_pcbListos* lista){
	pthread_mutex_lock(&lista->mutex);
	int i;
	int a;
	t_pcb *pcbAux;
	t_pcb *pcb;
	t_list *listaAux;
	listaAux = list_take_and_remove(lista->listos, lista->listos->elements_count);
	for (i=0; i < listaAux->elements_count; i++) {

		pcbAux = (t_pcb*) list_get(listaAux,i);
		if(list_is_empty(lista->listos))
			list_add(lista->listos, pcbAux);
		else {
			for (a=0; a<lista->listos->elements_count; a++){
				pcb = (t_pcb*)list_get(lista->listos,a);
				if(pcb->instante_llegada >= pcbAux->instante_llegada)
					break;
				}
			if (lista->listos->elements_count==a){
				list_add(lista->listos,pcbAux);
			}else{
				list_add_in_index(lista->listos,a,pcbAux);
			}
		}

	}
	list_destroy(listaAux);
	pthread_mutex_unlock(&lista->mutex);
}
