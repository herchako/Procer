/*
 * pcb.h
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */

#ifndef PCB_H_
#define PCB_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"

extern double rafaga_estimada;
extern double rafaga_real;

typedef struct {
	int16_t pid;
	t_list *seg_datos;
	t_list *seg_pila;
	char *seg_codigo;
	t_list *pos_fun;
	t_list *pos_etq;
	int64_t instante_llegada;
	int8_t prioridad;
	int8_t estado;
	int16_t socket_id;
	int32_t linea_actual;
	double ult_rafaga;
	double raf_actual;
} t_pcb;

typedef struct {
	char *variable;
	int32_t valor;
} t_seg_datos;

typedef struct {
	int32_t linea;
	char *nomb_fun;
} t_pos_fun;

typedef struct {
	int32_t linea;
	char *nomb_etq;
} t_pos_etq;

typedef struct {
    int32_t linea;
    char *llamada_funcion;
} t_seg_pila;

char* sub_string(const char* str, size_t begin, size_t len);
int load_file_to_memory(const char *filename, char **result);
int obtener_cantidad_lineas (char *result);
char* obtenerLinea(char*contenido, int numLinea);
void limpiarCodigo(char *codigo, char caracter);
int cargar_pcb(const char *filename , t_pcb *pcb);
int8_t mostrar_pcb (t_pcb * pcb);
int isNumeric(const char *val);
int32_t buscar_linea_funcion (t_pcb * pcb, char * nombre_fun);
int32_t buscar_linea_etiqueta (t_pcb * pcb, char * nombre_etq);
int32_t valor (t_pcb * pcb, char * nombre_var);
int8_t reemplazar_valor_seg_datos (t_pcb * pcb, char * nombre_var, int32_t valor);
int ejecutar (t_pcb *pcb, int32_t *tiempo, char**variableIO);
int8_t destruir_pcb (t_pcb *pcb);
char *guardar_estado_pcb(t_pcb *pcb);
int numDigits(int64_t numero);




#endif /* PCB_H_ */
