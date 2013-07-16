/*
 * listSTS.h
 *
 *  Created on: 03/11/2012
 *      Author: utnso
 */

#ifndef LISTSTS_H_
#define LISTSTS_H_

#include <stdint.h>
#include "collections/queue.h"
#include "pcb.h"
#include <pthread.h>
#include "collections/list.h"

typedef struct {
	t_queue* queue;
	pthread_mutex_t mutex;
} t_queueSTS;

typedef struct {
	t_list *listos;
	pthread_mutex_t mutex;
} t_pcbListos;

typedef struct {
	t_pcb *pcb;
	unsigned int tiempo_io;
} t_io;

t_queueSTS *queueSTS_create();
t_pcbListos *pcbListos_create();
void queueSTS_destroy(t_queueSTS *queue);
void queueBloqueados_destroy(t_queueSTS *queue);
void pcbListos_destroy(t_pcbListos *listos);
t_io *pop_queueBloqueados(t_queueSTS *queue);
void push_queueSTS (t_queueSTS* queue, t_pcb* pcb);
void push_queueBloqueados(t_queueSTS* queue, t_pcb* pcb, unsigned int tiempo);
void insertar_ordenado_FIFO(t_pcbListos* lista, t_pcb *pcb);
void insertar_ordenado_PRI(t_pcbListos* lista, t_pcb *pcb);
void insertar_ordenado_SPN(t_pcbListos* lista, t_pcb *pcb);
void reordenar_a_PRI (t_pcbListos* lista);
void reordenar_a_SPN (t_pcbListos* lista);
void reordenar_a_FIFO (t_pcbListos* lista);
extern double valor_alfa;


#endif /* LISTSTS_H_ */
