/*
 * pcb.c
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */
#define _GNU_SOURCE
#include "serialization.h"
#include "pcb.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"
#include <ctype.h>
#include <unistd.h>






t_stream * deserealizar_stream (char * buffer){
	t_stream *stream = malloc(sizeof(t_stream));
	int offset=0,i=0,tmp_size=0;
	stream->packageType = atoi(&buffer[0]);
	stream->length = atoi(&buffer[2]);
	for(i=0; buffer[i] != '#'; i++);
	i++;
	while(buffer[i] != '#') {
		i++;
	}
	i++;
	offset+=i;
	for(tmp_size = 1; (buffer + offset)[tmp_size - 1] != '\0'; tmp_size++);
	asprintf(&stream->data, "%s", buffer + offset );
	return stream;
}




void free_package_file (t_file_package *self)
{

    free(self->code);
    free(self);
}
void free_stream (t_stream *self)
{
    free(self->data);
    free(self);
}
void free_buffer(char * self)
{
    free(self);
}

t_file_package * create_package_file ( uint32_t file_desc,int16_t priority,char *code)
{
    t_file_package *paq = malloc(sizeof(t_file_package));
    paq->priority = priority;
    paq->file_desc = file_desc;
    paq->code = malloc(strlen(code)+1);
    strcpy(paq->code,code);
    return paq;
}


int obtener_tamanio_paquete(char *paquete){
	return atoi(&paquete[2]);
	}

int obtener_tipo_paquete(char *paquete){
	return atoi(&paquete[0]);
	}
int obtener_tamanio_prefijo_payload(char *paquete){
	int32_t i=0;
	for(i=0; paquete[i] != '#'; i++);

		i++;
		while(paquete[i] != '#') {
			i++;
		}
	i++;
	return i;
}


char *serializar_arch(t_file_package *paq) {

	char *payLoad;// = malloc(numDigits(paq->file_desc)+numDigits(paq->priority)+strlen(paq->code)+2);
	asprintf(&payLoad, "%d#%d#%s", (uint32_t)paq->file_desc,(int16_t)paq->priority, paq->code);
	char *pack;// = malloc(numDigits(PROCESO) + numDigits((int64_t)strlen(payLoad)) + strlen(payLoad)+2);

	asprintf(&pack, "%d#%d#%s",PROCESO, strlen(payLoad), payLoad);
	free(payLoad);
	return pack;
}



char *serializar_ope(char *info_operacion, int8_t operacion) { //el paquete se serializa con el tipo de operacion que es

	char *payLoad;// = malloc(strlen(info_operacion));
	asprintf(&payLoad, "%s",  info_operacion);
	char *pack;// = malloc(numDigits( operacion)+ numDigits( strlen(payLoad)) + strlen(payLoad)+2);

	asprintf(&pack, "%d#%d#%s", (int8_t)operacion, strlen(payLoad), payLoad);
	free(payLoad);
	return pack;

}


t_file_package * package_file_deserializer(t_stream *stream)
{

	t_file_package *arch = malloc(sizeof(t_file_package));
	int offset=0,i=0,tmp_size=0;
	char *aux;
	arch->file_desc = atoi(&stream->data[0]);
	arch->priority = atoi(&stream->data[2]);
	for(i=0; stream->data[i] != '#'; i++);
	i++;
	while(stream->data[i] != '#') {
		i++;
	}
	i++;
	offset+=i;
	for(i=offset,tmp_size=0; stream->data[i] != '\0'; i++,tmp_size++);
	aux=sub_string(stream->data,offset,tmp_size);
	asprintf(&arch->code, "%s",(char *)aux);
	free(aux);

	return arch;

}




