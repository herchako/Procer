/*
 * pcb.h
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */

#ifndef SERIALIZATION_H_
#define SERIALIZATION_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"
#include <stdint.h>


#define PROCESO         1//Paquete que envia un proceso ~ cliente a servidor
#define SUSPENSION      2//Paquete que avisa de la suspension ~  servidor a cliente
#define REANUDACION     3//Paquete que avisa de la reanudacion ~ cliente a servidor
#define FIN_PROCESO     4//Paquete que avisa del fin exitoso de un proceso  ~  servidor a cliente
#define EJEC_DEMORADA   5//Paquete que avisa de la ejecucion demorada por superar MMP ~    servidor a cliente
#define DENEGACION_EJEC 6//Paquete que avisa de la no ejecucion por superar MPS ~  servidor a cliente
#define HANDSHAKE       7
#define RTA_HANDSHAKE   8
#define IMPRIMIR_VAR    9//Paqeute que envia el nombre y estado de la variable ~ servidor a cliente

typedef struct {
    int8_t packageType;
    int32_t length;
    char *data;
}  t_stream;

typedef struct{
	uint32_t file_desc;
	int16_t priority;
	char *code;
}  t_file_package;




t_stream * deserealizar_stream (char * buffer);
void free_package_file (t_file_package *self);
void free_stream (t_stream *self);
void free_buffer(char * self);
t_file_package * create_package_file ( uint32_t file_desc,int16_t priority,char *code);
int obtener_tamanio_paquete(char *paquete);
int obtener_tipo_paquete(char *paquete);
int obtener_tamanio_prefijo_payload(char *paquete);
char *serializar_arch(t_file_package *paq);
char *serializar_ope(char *info_operacion, int8_t operacion);
t_file_package * package_file_deserializer(t_stream *stream);



#endif /* SERIALIZATION_H_ */
