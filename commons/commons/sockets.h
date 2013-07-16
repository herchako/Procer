/*
 * pcb.h
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"
#include "pcb.h"
#include <stdint.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <termios.h>

#include <sys/socket.h>

#define MAX_EVENTS 20
#define MIN_CONEXIONES 5

extern int16_t puertoHost;


uint32_t conectar_cliente(uint32_t *fdCliente, uint32_t puerto, char* ip);
uint32_t iniciar_conexion(char* ip, uint32_t puerto);
char *recivirBytes(int socketE , int bytesCount, uint8_t * error);
int32_t levantar_server(int32_t *socket);
int enviarScript(uint32_t fd_client,uint32_t fd_enserver, int16_t priority,char *codigo);
int enviar_variable (t_pcb *pcb,  char *var, int32_t valor);



#endif /* SOCKETS_H_ */
