/*
 * pcb.c
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */

#include "pcb.h"

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"
#include <ctype.h>

//TODO VERIFICAR Q ESTADOS SEAN CORRECTOS
#define NEW    0
#define READY  1
#define EXEC   2
#define BLOCK  3
#define FINISH 4
#define SUSP   5


/*DECLARACIONES STATIC*/
static t_seg_datos *crear_nodo_datos(int32_t valor, char *variable);
static void destruir_nodo_datos(t_seg_datos *self);
static t_pos_fun *crear_nodo_funcion(int32_t linea, char *funcion);
static void destruir_nodo_funcion(t_pos_fun *self);
static t_pos_etq *crear_nodo_etiqueta(int32_t linea, char *etiqueta);
static void destruir_nodo_etiqueta(t_pos_etq *self);
static t_seg_pila *crear_nodo_pila(int32_t linea, char *funcion);
static void destruir_nodo_pila(t_seg_pila *self);

char* sub_string(const char* str, size_t begin, size_t len) {
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin
			|| strlen(str) < (begin + len))
		return 0;

	return strndup(str + begin, len);
}

int load_file_to_memory(const char *filename, char **result) {
	int32_t tam_arch = 0;
	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		*result = NULL;
		return EXIT_FAILURE; //error apertura

	}
	fseek(f, 0, SEEK_END);
	tam_arch = ftell(f);
	fseek(f, 0, SEEK_SET);
	*result = (char *) malloc(tam_arch + 1);
	if (tam_arch != fread(*result, sizeof(char), tam_arch, f)) {
		free(*result);
		return EXIT_FAILURE; //error lectura

	}
	fclose(f);
	(*result)[tam_arch] = 0;
	return tam_arch;
}

int obtener_cantidad_lineas(char *result) {
	int32_t cantCaracteres = strlen(result), i = 0, cantLineas = 0;
	while (i < cantCaracteres) {
		if (result[i] == '\n') {
			cantLineas++;
		}
		i++;
	}
	return cantLineas;
}

char* obtenerLinea(char*contenido, int numLinea) {
	int32_t lineaActual = 0, comienzoLinea = 0, caracterActual = 0; // línea

	char *cadena;

	while (lineaActual < numLinea) {
		comienzoLinea = caracterActual;
		while (contenido[caracterActual] != '\n') {
			caracterActual++;
		}
		caracterActual++;
		lineaActual++;
	}
	cadena = sub_string(contenido, comienzoLinea,
			caracterActual - comienzoLinea);

	return cadena;
}

void limpiarCodigo(char *codigo, char caracter) {
	int32_t i, j;
	for (i=0; *(codigo+i) != '\0'; i++)
	{
		if (*(codigo+i) == caracter)
		{
			for (j=i; *(codigo+j) != '\0'; j++)
				*(codigo+j) = *(codigo+j+1);
		}
	}
	if (*(codigo+i-1) != '\n')
	{
		*(codigo+i) = '\n';
		*(codigo+i+1) = '\0';
	}
}

int cargar_pcb(const char *filename, t_pcb *pcb) {


	int32_t tam_arch;

	int32_t pos_car_act = 0; //posicion en la linea

	int32_t i = 0;

	int32_t comentario = 1;

	pcb->pid = random();
	pcb->seg_datos = list_create();
	pcb->seg_pila = list_create();
	pcb->pos_fun = list_create();
	pcb->pos_etq = list_create();
	pcb->instante_llegada = 0;
	pcb->prioridad = random();
	pcb->estado = NEW;
	pcb->socket_id = random();
	pcb->linea_actual = 1; //TODO no arranco definirlo con nacho
	pcb->ult_rafaga = rafaga_real;
	pcb->raf_actual = rafaga_estimada;

	tam_arch = load_file_to_memory(filename, &(pcb->seg_codigo)); // aca se sube a memoria, es imporante entender que content ahora, es un "vector" de una sola dimencion que tiene todas las lineas separadas por '\n'

	if (tam_arch < 0) {
		puts("Error loading file");
		return 1;
	}
	char *aux;
	char *subCadena;
	char ** subString;
	int32_t declaracionVariables = 1, declaracionFuncion = 1;
	int32_t ultimoCaracter = 0;

	for (i = 1; i <= obtener_cantidad_lineas(pcb->seg_codigo); i++) {
//		printf("Línea: %d\n", i);
		aux = obtenerLinea(pcb->seg_codigo, i);
		for (pos_car_act = 0; pos_car_act <= ((strlen(aux))); pos_car_act++) {
			//			printf("carácter: %c\n", *((aux->linea)+pos_car_act));
			if (*(aux + pos_car_act) == '#')
				comentario = 0;

			if (comentario == 1) {

				switch (*(aux + pos_car_act)) {
				case ' ':
					subCadena = sub_string(aux, 0, pos_car_act);
					//				printf("subCadena: %s\n", subCadena);
					if ((strcmp(subCadena, "variables")) == 0) {
						declaracionVariables = 0;
						ultimoCaracter = pos_car_act;
					}
					if ((strcmp(subCadena, "comienzo_funcion")) == 0) {
						declaracionFuncion = 0;
						//					printf("Comienza una función: ");
						ultimoCaracter = pos_car_act;
					}
					free(subCadena);
					break;
				case ',':
					if (declaracionVariables == 0) {
						subCadena = sub_string(aux, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						//					printf("subCadena: %s\n", subCadena);
						list_add(pcb->seg_datos,
								crear_nodo_datos(0, subCadena));
						//					printf("Variable: %s, valor: %d\n", subCadena, valor(pcb, subCadena));
						ultimoCaracter = pos_car_act;
						free(subCadena);
					}
					break;
				case ':':
					subCadena = sub_string(aux, 0, pos_car_act);
					//				printf("subCadena: %s\n", subCadena);
					list_add(pcb->pos_etq, crear_nodo_etiqueta(i, subCadena));
					//				printf("Etiqueta: %s \n", subCadena);
					free(subCadena);
					break;
				case '\n':
					if (declaracionVariables == 0) // me encuentro en una declaración de variables
							{
						subCadena = sub_string(aux, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						//					printf("subCadena: %s\n", subCadena);
						if (strchr(subCadena, ';') != NULL) {
							subString = string_split(subCadena, ";");
							strcpy(subCadena, subString[0]);
							free(subString[0]);
							free(subString[1]);
							free(subString);
						}
						list_add(pcb->seg_datos,
								crear_nodo_datos(0, subCadena));

						//					printf("Variable: %s, valor: %d\n", subCadena, valor(pcb, subCadena));

						declaracionVariables = 1;
						free(subCadena);

					} else if (declaracionFuncion == 0) {
						subCadena = sub_string(aux, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						//						printf("%s\n", subCadena);
						if (strchr(subCadena, ';') != NULL) {
							subString = string_split(subCadena, ";");
							strcpy(subCadena, subString[0]);
							free(subString[0]);
							free(subString[1]);
							free(subString);
						}
						list_add(pcb->pos_fun,
								crear_nodo_funcion(i, subCadena));
						declaracionFuncion = 1;
						free(subCadena);
					}

					break;
				}
			}
		}
		free(aux);
		comentario = 1;
	}

	return EXIT_SUCCESS;
}

int8_t mostrar_pcb(t_pcb * pcb) {

	int32_t i = 0;

	printf("Segmento de código:\n");
	puts(pcb->seg_codigo);

	printf("\nSegmento de datos:\n");
	t_seg_datos *aux2;
	i = 0;
	while (i < pcb->seg_datos->elements_count) {
		aux2 = list_get(pcb->seg_datos, i);
		printf("%s vale: %d\n", aux2->variable, aux2->valor);
		i++;
	}

	printf("\nDefiniciones de funciones:\n");
	t_pos_fun *aux3;
	i = 0;
	while (i < pcb->pos_fun->elements_count) {
		aux3 = list_get(pcb->pos_fun, i);
		printf("Se declara %s en la línea %d\n", aux3->nomb_fun, aux3->linea);
		i++;
	}

	printf("\nEtiquetas:\n");
	t_pos_etq *aux4;
	i = 0;
	while (i < pcb->pos_etq->elements_count) {
		aux4 = list_get(pcb->pos_etq, i);
		printf("Etiqueta %s en la línea %d\n", aux4->nomb_etq, aux4->linea);
		i++;
	}

	printf("\nEstructura de stack:\n");
	t_seg_pila *aux5;
	i = 0;
	while (i < pcb->seg_pila->elements_count) {
		aux5 = list_get(pcb->seg_pila, i);
		printf("%d,%s\n", aux5->linea, aux5->llamada_funcion);
		i++;
	}
//	free(aux2);
//	free(aux3);
//	free(aux4);
//	free(aux5);

	printf("\n\nPID:%d\n", pcb->pid);
	printf("PRIORIDAD:%d\n", pcb->prioridad);
	printf("ESTADO:%d\n", pcb->estado);
	printf("SOCKET_ID:%d\n", pcb->socket_id);
	printf("LINEA_ACTUAL:%d\n", pcb->linea_actual);
	printf("ULTIMA_RAFAGA:%f\n", pcb->ult_rafaga);
	printf("RAFAGA_ACTUAL:%f\n", pcb->raf_actual);
	return EXIT_SUCCESS;
}

int isNumeric(const char *val) {
	int i = 0, s = 1;

	for (i = 0; i < strlen(val); i++)
		if (isdigit(val[i]))
			s = 0;

	return s;
}

int32_t buscar_linea_funcion(t_pcb * pcb, char * nombre_fun) {

	int32_t i = 0;
	t_pos_fun * aux_pos_fun;

	aux_pos_fun = list_get(pcb->pos_fun, i);
	while ((i < pcb->pos_fun->elements_count)
			&& (strcmp(aux_pos_fun->nomb_fun, nombre_fun) != 0)) {
		i++;
		aux_pos_fun = list_get(pcb->pos_fun, i);
	}
	if (i == pcb->pos_fun->elements_count) {
		printf("error función no encontrada");
		return -EXIT_FAILURE;
	}
	if (strcmp(aux_pos_fun->nomb_fun, nombre_fun) == 0) {
		return aux_pos_fun->linea;
	}

	return -EXIT_FAILURE;
}

int32_t buscar_linea_etiqueta(t_pcb * pcb, char * nombre_etq) {
	int32_t i = 0;
	t_pos_etq * aux_pos_etq;

	aux_pos_etq = list_get(pcb->pos_etq, i);
	while ((i < pcb->pos_etq->elements_count)
			&& (strcmp(aux_pos_etq->nomb_etq, nombre_etq) != 0)) {
		i++;
		aux_pos_etq = list_get(pcb->pos_etq, i);
	}
	if (i == pcb->seg_datos->elements_count) {
		printf("error etiqueta no encontrada");
		return -EXIT_FAILURE;
	}
	if (strcmp(aux_pos_etq->nomb_etq, nombre_etq) == 0) {
		return aux_pos_etq->linea;
	}

	return -EXIT_FAILURE;
}

int32_t valor(t_pcb * pcb, char * nombre_var) {

	int32_t i = 0, valorVariable;
	t_seg_datos * aux_datos;

	aux_datos = list_get(pcb->seg_datos, i);
	while ((i < pcb->seg_datos->elements_count)
			&& (strcmp(aux_datos->variable, nombre_var) != 0)) {
		i++;
		aux_datos = list_get(pcb->seg_datos, i);
	}
	if (i == pcb->seg_datos->elements_count) {
		printf("error variable no encontrada"); //todo loguear error
		return -1;
	}

	if (strcmp(aux_datos->variable, nombre_var) == 0) {
		valorVariable = aux_datos->valor;
		return valorVariable;
	}

	return 0;
}

int8_t reemplazar_valor_seg_datos(t_pcb * pcb, char * nombre_var, int32_t valor) {

	int32_t i = 0;
	t_seg_datos * aux_datos;

	aux_datos = list_get(pcb->seg_datos, i);
	while ((i < pcb->seg_datos->elements_count)
			&& (strcmp(aux_datos->variable, nombre_var) != 0)) {
		i++;
		aux_datos = list_get(pcb->seg_datos, i);
	}
	if (i == pcb->seg_datos->elements_count) {
		printf("error variable no encontrada"); //todo loguear error
		return EXIT_FAILURE;
	}
	if (strcmp(aux_datos->variable, nombre_var) == 0) {
		aux_datos->valor = valor;
		list_replace(pcb->seg_datos, i, aux_datos);

	}
	return EXIT_SUCCESS;
}

int8_t destruir_pcb(t_pcb *pcb) {

	free(pcb->seg_codigo);
	list_destroy_and_destroy_elements(pcb->seg_datos,
			(void*) destruir_nodo_datos);
	list_destroy_and_destroy_elements(pcb->pos_fun,
			(void*) destruir_nodo_funcion);
	list_destroy_and_destroy_elements(pcb->pos_etq,
			(void*) destruir_nodo_etiqueta);

	list_destroy_and_destroy_elements(pcb->seg_pila,
			(void*) destruir_nodo_pila);
//	free(pcb);
	return EXIT_SUCCESS;
}

int ejecutar(t_pcb *pcb, int32_t *tiempo, char**variableIO) {

	int32_t pos_car_act = 0; //posicion en la linea

	char **subString;
	char *subCadena;

	// Flags que indican qué hace la línea que se está leyendo:
	int32_t asignacionVariable = 1, suma = 1, resta = 1, imprimir = 1, snc = 1,
			ssc = 1, buscarEtiqueta = 1, programa = 1, io = 1, instruccion = 1,
			comentario = 1;
	int32_t ultimoCaracter;

	// Para asignación de variables
	int32_t valorVariable;

	// Para funciones y etiquetas:
	int32_t linea_salto;

	// Para saltar a una etiqueta:
	char*etiqueta;

	// Para operaciones aritméticas:
	char*operando1;
	char*operando2;

	int32_t op1, op2; // enteros para realizar operaciones aritméticas

	int32_t resultado;

	// Para la función io(parametro1, parametro2):
	char*parametro1;
	char*parametro2;

	int32_t p1, p2;

	t_seg_pila *primerElemento;
	t_seg_pila *nodo_pila;

	char *cadena;
	char *variable;
	char *valorAsignado;

	while ((programa != 2) && (instruccion == 1)) {
		printf("pcb->linea_actual: %d\n", pcb->linea_actual);
		char * linea = obtenerLinea(pcb->seg_codigo, pcb->linea_actual);
		if (strchr(linea, ';') != NULL) {
			subString = string_split(linea, ";");
			*tiempo = atoi(subString[1]);
			printf("La sentencia se temporizará en %d segundos.\n", *tiempo);
			free(subString[0]);
			free(subString[1]);
			free(subString);
		} else
			*tiempo = 0;

		for (pos_car_act = 0; pos_car_act < ((strlen(linea))); pos_car_act++) {
			if (*(linea + pos_car_act) == '#')
				comentario = 0;

			if (comentario == 1) {
				switch (*(linea + pos_car_act)) {
				case '\n':
					cadena = sub_string(linea, 0, pos_car_act);

					if ((strcmp(cadena, "comienzo_programa")) == 0)
						programa = 0;

					if ((strcmp(cadena, "fin_programa")) == 0)
						programa = 2;

					if (imprimir == 0) {
						subCadena = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						printf("IMPRIMIENDO VARIABLE %s: %d\n", subCadena,
								valor(pcb, subCadena));
						imprimir = 1;
						free(subCadena);
						instruccion = 0;
					}

					if (io == 0)
						instruccion = 0;

					if (asignacionVariable == 0) {
						if (io == 1) {
							if (suma == 1 && resta == 1) {
								valorAsignado = sub_string(linea,
										ultimoCaracter + 1,
										pos_car_act - ultimoCaracter - 1);
								if (isNumeric(valorAsignado) == 0) // es un número
										{
									valorVariable = atoi(
											(const char*) valorAsignado);
									reemplazar_valor_seg_datos(pcb, variable,
											valorVariable); // ¿se puede hacer esto?
								} else // es una variable
								{
									valorVariable = valor(pcb, cadena);
									reemplazar_valor_seg_datos(pcb, variable,
											valorVariable);
								}
								free(valorAsignado);
							}

							if (suma == 0) {
								operando2 = sub_string(linea,
										ultimoCaracter + 1,
										pos_car_act - ultimoCaracter - 1);
								if (tiempo != 0) {
									subString = string_split(operando2, ";");
									strcpy(operando2, subString[0]);
									free(subString[0]);
									free(subString[1]);
									free(subString);
								}

								if (isNumeric(operando1) != 0)
									op1 = valor(pcb, operando1);

								else
									op1 = atoi(operando1);

								if (isNumeric(operando2) != 0)
									op2 = valor(pcb, operando2);

								else
									op2 = atoi(operando2);

								resultado = op1 + op2;
								reemplazar_valor_seg_datos(pcb, variable,
										resultado);
								suma = 1;
								free(operando1);
								free(operando2);
							}

							if (resta == 0) {
								operando2 = sub_string(linea,
										ultimoCaracter + 1,
										pos_car_act - ultimoCaracter - 1);
								if (*tiempo != 0) {
									subString = string_split(operando2, ";");
									strcpy(operando2, subString[0]);
									free(subString[0]);
									free(subString[1]);
									free(subString);
								}

								if (isNumeric(operando1) != 0)
									op1 = valor(pcb, operando1);
								else
									op1 = atoi(operando1);

								if (isNumeric(operando2) != 0) {
									op2 = valor(pcb, operando2);
								} else
									op2 = atoi(operando2);

								resultado = op1 - op2;
								reemplazar_valor_seg_datos(pcb, variable,
										resultado);
								resta = 1;
								free(operando1);
								free(operando2);
							}
						} else
							strcpy(variableIO[0], variable);

						free(variable);
						asignacionVariable = 1;
						instruccion = 0;
					}

					if (buscarEtiqueta == 0) {
						etiqueta = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						linea_salto = buscar_linea_etiqueta(pcb, etiqueta);
						pcb->linea_actual = linea_salto;
						buscarEtiqueta = 1;
						snc = 1;
						ssc = 1;
						free(etiqueta);
						free(variable);
						instruccion = 0;
					}
					free(cadena);
					break;

				case '=':
					asignacionVariable = 0;
					variable = sub_string(linea, 0, pos_car_act);
					ultimoCaracter = pos_car_act;

					break;

				case '+':
					suma = 0;
					operando1 = sub_string(linea, ultimoCaracter + 1,
							pos_car_act - ultimoCaracter - 1);
					ultimoCaracter = pos_car_act;
					break;

				case '-':
					resta = 0;
					operando1 = sub_string(linea, ultimoCaracter + 1,
							pos_car_act - ultimoCaracter - 1);
					ultimoCaracter = pos_car_act;
					break;

				case '(':
					if (asignacionVariable == 0)
						cadena = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
					else
						cadena = sub_string(linea, 0, pos_car_act);

					if ((strcmp(cadena, "io")) == 0) {
						io = 0;
						ultimoCaracter = pos_car_act;
					} else {
						nodo_pila = crear_nodo_pila(pcb->linea_actual, cadena);
						if (list_is_empty(pcb->seg_pila))
							list_add(pcb->seg_pila, nodo_pila);
						else
							list_add_in_index(pcb->seg_pila, 0, nodo_pila);

						linea_salto = buscar_linea_funcion(pcb, cadena);
						pcb->linea_actual = linea_salto;
					}
					free(cadena); // GUARDA
					break;
				case ',':
					if (io == 0) {
						parametro1 = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						ultimoCaracter = pos_car_act;
					}
					break;
				case ')':
					if (io == 0) {
						parametro2 = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						p1 = atoi(parametro1);
						p2 = atoi(parametro2);
						*tiempo = p1;
//							printf("Se accederá al dispositivo durante %d segundos. ", *tiempo);
						free(parametro1);
						free(parametro2);
					}
					break;
				case ' ':
					subCadena = sub_string(linea, 0, pos_car_act);
					if ((strcmp(subCadena, "imprimir")) == 0) {
						imprimir = 0;
						ultimoCaracter = pos_car_act;
					}

					if ((strcmp(subCadena, "fin_funcion")) == 0) {
						primerElemento = list_get(pcb->seg_pila, 0);
						linea_salto = primerElemento->linea;
						pcb->linea_actual = linea_salto;
						free(primerElemento);
						list_remove(pcb->seg_pila, 0);
					}

					if (snc == 0) {
						variable = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						if ((valor(pcb, variable)) != 0) {
							buscarEtiqueta = 0;
							ultimoCaracter = pos_car_act;
						} else {
							snc = 1;
							free(variable);
						}

					}

					if (ssc == 0) {
						variable = sub_string(linea, ultimoCaracter + 1,
								pos_car_act - ultimoCaracter - 1);
						if ((valor(pcb, variable)) != 0) {
							buscarEtiqueta = 0;
							ultimoCaracter = pos_car_act;
						} else {
							ssc = 1;
							free(variable);
						}
					}

					if ((strcmp(subCadena, "snc")) == 0) {
						snc = 0;
						ultimoCaracter = pos_car_act;
					}

					if ((strcmp(subCadena, "ssc")) == 0) {
						ssc = 0;
						ultimoCaracter = pos_car_act;
					}

					free(subCadena);
					break;
				}
			}
		}
		free(linea);
		(pcb->linea_actual)++;
		comentario = 1;
	}

	if (programa == 2)
		return 0; // terminó el programa

	if (io == 0) {
		if (p2 == 1)
			return 1; // IO bloqueante
		else
			return 2; // IO no bloqueante
	}

	return 3; // ejecutó la instrucción correctamente y no era ni finalizción del programa ni I/O
}

static t_seg_datos *crear_nodo_datos(int32_t valor, char *variable) {
	t_seg_datos *new = malloc(sizeof(t_seg_datos));
	new->variable = strdup(variable);
	new->valor = valor;
	return new;
}

static void destruir_nodo_datos(t_seg_datos *self) {
	free(self->variable);
	free(self);
}

static t_pos_fun *crear_nodo_funcion(int32_t linea, char *funcion) {
	t_pos_fun *new = malloc(sizeof(t_pos_fun));
	new->nomb_fun = strdup(funcion);
	new->linea = linea;
	return new;
}

static void destruir_nodo_funcion(t_pos_fun *self) {
	free(self->nomb_fun);
	free(self);
}

static t_pos_etq *crear_nodo_etiqueta(int32_t linea, char *etiqueta) {
	t_pos_etq *new = malloc(sizeof(t_pos_etq));
	new->nomb_etq = strdup(etiqueta);
	new->linea = linea;
	return new;
}

static void destruir_nodo_etiqueta(t_pos_etq *self) {
	free(self->nomb_etq);
	free(self);
}

static t_seg_pila *crear_nodo_pila(int32_t linea, char *funcion) {
	t_seg_pila *new = malloc(sizeof(t_seg_pila));
	new->llamada_funcion = strdup(funcion);
	new->linea = linea;
	return new;
}

void destruir_nodo_pila(t_seg_pila *self) {
	free(self->llamada_funcion);
	free(self);
}

int numDigits(int64_t numero) {
	int digits = 0;
	if (numero < 0)
		digits = 1;
	while (numero) {
		numero /= 10;
		digits++;

	}
	return digits;
}


char *guardar_estado_pcb(t_pcb *pcb) {
	int cantidad = 0;
	int i;
	cantidad = strlen("ID=\n") + 1 + numDigits(pcb->pid)
			+ numDigits(pcb->linea_actual) + strlen("PC=\n") + 1
			+ strlen("\nEstructura de Codigo\n") + 1 + strlen(pcb->seg_codigo)
			+ 1 + strlen("\nEstructura de Datos\n") + 1
			+ strlen("\nEstructura de Stack\n") + 1;
	for (i = 0; i < pcb->seg_datos->elements_count; i++) {
		t_seg_datos *seg = list_get(pcb->seg_datos, i);
		cantidad = cantidad + strlen(seg->variable) + 1 + strlen("=") + 1
				+ numDigits(seg->valor) + strlen("\n") + 1;
	}
	for (i = 0; i < pcb->seg_pila->elements_count; i++) {
		t_seg_pila *pila = list_get(pcb->seg_pila, i);
		cantidad = cantidad + numDigits(pila->linea) + strlen(",") + 1
				+ strlen(pila->llamada_funcion) + 1 + strlen("\n") + 1;
	}
	char *estado = malloc(cantidad);

	sprintf(estado, "ID=%d\nPC=%d\n\nEstructura de Codigo\n%s", pcb->pid,
			pcb->linea_actual, pcb->seg_codigo);
	sprintf(estado, "%s%s", estado, "\nEstructura de Datos\n");
	for (i = 0; i < pcb->seg_datos->elements_count; i++) {
		t_seg_datos *seg = list_get(pcb->seg_datos, i);
		sprintf(estado, "%s%s%s%d\n", estado, seg->variable, "=", seg->valor);
	}
	sprintf(estado, "%s%s", estado, "\nEstructura de Pila\n");
	for (i = 0; i < pcb->seg_pila->elements_count; i++) {
		t_seg_pila *pila = list_get(pcb->seg_pila, i);
		sprintf(estado, "%s%d%s%s\n", estado, pila->linea, ",",
				pila->llamada_funcion);
	}
	return estado;

}
