/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "log.h"

#include "temporal.h"
#include "error.h"
#include "string.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>


#ifndef LOG_MAX_LENGTH_MESSAGE
#define LOG_MAX_LENGTH_MESSAGE 1024
#endif

#define LOG_MAX_LENGTH_BUFFER LOG_MAX_LENGTH_MESSAGE + 100
#define LOG_ENUM_SIZE 6

static char *enum_names[LOG_ENUM_SIZE] = {"TRACE", "DEBUG", "INFO", "WARNING", "ERROR", "LSCH"};

/**
 * Private Functions
 */
static void log_write_in_level(t_log* logger, t_log_level level, const char* message_template, va_list arguments);
static bool isEnableLevelInLogger(t_log* logger, t_log_level level);

/**
 * Public Functions
 */


/**
 * @NAME: log_create
 * @DESC: Crea una instancia de logger, tomando por parametro
 * el nombre del programa, el nombre del archivo donde se van a generar los logs,
 * el nivel de detalle minimo a loguear y si además se muestra por pantalla lo que se loguea.
 */
t_log* log_create(char* file, char *program_name, bool is_active_console, t_log_level detail) {
	t_log* logger = malloc(sizeof(t_log));

	if (logger == NULL) {
		perror("Cannot create logger");
		return NULL;
	}

	FILE *file_opened = NULL;

	if (file != NULL) {
		file_opened = fopen(file, "a");

		if (file_opened == NULL) {
			perror("Cannot create/open log file");
			free(logger);
			return NULL;
		}
	}

	logger->file = file_opened;
	logger->is_active_console = is_active_console;
	logger->detail = detail;
	logger->pid = getpid();
	logger->program_name = strdup(program_name);
	return logger;
}


/**
 * @NAME: log_destroy
 * @DESC: Destruye una instancia de logger
 */
void log_destroy(t_log* logger) {
	free(logger->program_name);
	fclose(logger->file);
	free(logger);
}

/**
 * @NAME: log_trace
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [TRACE] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_trace(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_TRACE, message_template, arguments);
	va_end(arguments);
}

/**
 * @NAME: log_debug
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [DEBUG] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_debug(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_DEBUG, message_template, arguments);
	va_end(arguments);
}

/**
 * @NAME: log_info
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [INFO] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_info(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_INFO, message_template, arguments);
	va_end(arguments);
}

/**
 * @NAME: log_warning
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [WARNING] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_warning(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_WARNING, message_template, arguments);
	va_end(arguments);
}

/**
 * @NAME: log_error
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [ERROR] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_error(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_ERROR, message_template, arguments);
	va_end(arguments);
}

/**
 * @NAME: log_lsch
 * @DESC: Loguea un mensaje con el siguiente formato
 *
 * [LSCH] hh:mm:ss:mmmm PROCESS_NAME/(PID:TID): MESSAGE
 *
 */
void log_lsch(t_log* logger, const char* message_template, ...) {
	va_list arguments;
	va_start(arguments, message_template);
	log_write_in_level(logger, LOG_LEVEL_LSCH, message_template, arguments);
	va_end(arguments);
}



/**
 * @NAME: log_level_as_string
 * @DESC: Convierte un t_log_level a su representacion en string
 */
char *log_level_as_string(t_log_level level) {
	return enum_names[level];
}

/**
 * @NAME: log_level_from_string
 * @DESC: Convierte un string a su representacion en t_log_level
 */
t_log_level log_level_from_string(char *level) {
	int i;

	for (i = 0; i < LOG_ENUM_SIZE; i++) {
		if (string_equals_ignore_case(level, enum_names[i])){
			return i;
		}
	}

	return -1;
}

int function_logueo(char *proceso, char *hilo, char *tipo, char msj[100]) {
	if (strcmp(proceso, "PI") == 0) {
		char path_log[100]; //Este es el path del archivo donde logueas
		sprintf(path_log, "/home/utnso/PI.%d.log", (int) getpid()); //en realidad esto es porque el nombre del archivo LOG del PI tiene que contener el PID del proceso PI, pero en el caso del pp podes directamente pasarle el path //sin necesidad del sprinft y del getpid. Podes hacer arriba char path_log[] = "/home/blabla"; y eliminar el sprintf
		FILE *archivo = fopen(path_log, "a"); //Te crea un archivo en el path. Si ya existe, no lo crea. El parametro a era por algo, averigualo
		if (strcmp(tipo, "INFO") == 0) {
			t_log *log_informacion = log_create(path_log, "PI_MedranoYLaBashe",	false, LOG_LEVEL_INFO); //Si queres loguear info, hay otras cosas para errores, warnings, etc.
			log_info(log_informacion, msj);
			log_destroy(log_informacion);
		} else if (strcmp(tipo, "ERROR") == 0) {
			t_log *log_err = log_create(path_log, "PI_MedranoYLaBashe", false,LOG_LEVEL_ERROR);
			log_error(log_err, msj);
			log_destroy(log_err);
		} else if (strcmp(tipo, "DEBUG") == 0) {
			t_log *log_deb = log_create(path_log, "PI_MedranoYLaBashe", false,LOG_LEVEL_DEBUG);
			log_debug(log_deb, msj);
			log_destroy(log_deb);
		} else {
			t_log *log_warn = log_create(path_log, "PI_MedranoYLaBashe", false, LOG_LEVEL_WARNING);
			log_warning(log_warn, msj);
			log_destroy(log_warn);
		}
		fclose(archivo);
	} else { //logueo del pp
		char path_log[100];
		sprintf(path_log, "/home/utnso/PP.log");
		FILE *archivo = fopen(path_log, "a");
		char program_name[100];
		if (hilo==0)
			sprintf(program_name, "PP_MedranoYLaBashe");
		else
			sprintf(program_name, "PP_MedranoYLaBashe/Hilo %s-TID", hilo);


		if (strcmp(tipo, "INFO") == 0) {
			t_log *log_informacion = log_create(path_log, program_name, false,LOG_LEVEL_INFO);
			log_info(log_informacion, msj);
			log_destroy(log_informacion);
		} else if (strcmp(tipo, "ERROR") == 0) {
			t_log *log_err = log_create(path_log, program_name, false,LOG_LEVEL_ERROR);
			log_error(log_err, msj);
			log_destroy(log_err);
		} else if (strcmp(tipo, "DEBUG") == 0) {
			t_log *log_deb = log_create(path_log, program_name, false,LOG_LEVEL_DEBUG);
			log_debug(log_deb, msj);
			log_destroy(log_deb);
		} else if (strcmp(tipo, "WARN")==0) {
			t_log *log_warn = log_create(path_log, program_name, false, LOG_LEVEL_WARNING);
			log_warning(log_warn, msj);
			log_destroy(log_warn);
		} else {
			t_log *log_list_change = log_create(path_log, program_name, false, LOG_LEVEL_LSCH);
			log_lsch(log_list_change, msj);
			log_destroy(log_list_change);
		}
		fclose(archivo);

	}

	return 0;
}

int function_logueo_lsch(int32_t pid, char *origen, char *destino, char *hilo, int cola,
		int16_t prioridad_lpn, int16_t prioridad_lpr, int16_t prioridad_lpfq,
		int16_t prioridad_lpfio) {
	char path_log[100];
	char msj[100];
	char program_name[50];
	sprintf(path_log, "/home/utnso/PP.log");
	FILE *archivo=fopen(path_log, "a");
	if (hilo==0)
		sprintf(program_name, "PP_MedranoYLaBashe");
	else
		sprintf(program_name, "PP_MedranoYLaBashe/Hilo %s-TID", hilo);

	if (origen == 0 && cola == -1) {
		sprintf(msj, "El proceso %d se ha insertado en %s", pid, destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);

	} else if (cola == -1) {
		sprintf(msj, "El proceso %d se sacado de %s y se ha insertado en %s",
				pid, origen, destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);
	} else if (prioridad_lpfio - 1 == cola) {
		sprintf(msj, "El proceso %d se sacado de %s y se ha insertado en %s",
				pid, "FinIO", destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);
	} else if (prioridad_lpfq - 1 == cola) {
		sprintf(msj, "El proceso %d se sacado de %s y se ha insertado en %s",
				pid, "FinQ", destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);
	} else if (prioridad_lpn - 1 == cola) {
		sprintf(msj, "El proceso %d se sacado de %s y se ha insertado en %s",
				pid, "Nuevos", destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);
	} else if (prioridad_lpr - 1 == 0) {
		sprintf(msj, "El proceso %d se sacado de %s y se ha insertado en %s",
				pid, "Reanudados", destino);
		t_log *log_list_change = log_create(path_log, program_name,
				false, LOG_LEVEL_LSCH);
		log_lsch(log_list_change, msj);
		log_destroy(log_list_change);
	}
	fclose(archivo);

	return 0;
}




/** PRIVATE FUNCTIONS **/

static void log_write_in_level(t_log* logger, t_log_level level, const char* message_template, va_list list_arguments) {

	if (isEnableLevelInLogger(logger, level)) {
		char *message, *time, *buffer;
		unsigned int thread_id;

		message = malloc(LOG_MAX_LENGTH_MESSAGE + 1);
		vsprintf(message, message_template, list_arguments);
		time = temporal_get_string_time();
		thread_id = pthread_self();

		buffer = malloc(LOG_MAX_LENGTH_BUFFER + 1);
		sprintf(buffer, "[%s] %s %s/(%d:%d): %s\n",
				log_level_as_string(level), time,
				logger->program_name, logger->pid, thread_id,
				message);

		if (logger->file != NULL) {
			fprintf(logger->file, "%s", buffer);
			fflush(logger->file);
		}

		if (logger->is_active_console) {
			printf("%s", buffer);
			fflush(stdout);
		}

		free(time);
		free(message);
		free(buffer);
	}
}

static bool isEnableLevelInLogger(t_log* logger, t_log_level level) {
	return level >= logger->detail;
}
