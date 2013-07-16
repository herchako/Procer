/*
 * pcb.c
 *
 *  Created on: 02/11/2012
 *      Author: ignacio-hernan
 */

#define _GNU_SOURCE
#include "sockets.h"


#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "collections/list.h"
#include <ctype.h>
#include <unistd.h>
#include "serialization.h"



uint32_t conectar_cliente(uint32_t *fd_pi, uint32_t puerto, char* ip) {

	struct sockaddr_in *server_addr = malloc(sizeof(struct sockaddr_in));

	if ((*fd_pi = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("No se pudo crear el socket");
		return EXIT_FAILURE;
	}

	server_addr->sin_family = AF_INET;
	server_addr->sin_addr.s_addr = inet_addr(ip);
	server_addr->sin_port = htons(puerto);
	//len = sizeof(struct sockaddr_in);todo

	if (connect(*fd_pi, (struct sockaddr *) server_addr, sizeof(struct sockaddr_in)) == -1) {
		perror("Error connect");
		return EXIT_FAILURE;
	}
	free(server_addr);
	return EXIT_SUCCESS;
}


uint32_t iniciar_conexion(char* ip, uint32_t puerto) {
	uint32_t fd;

	char *handshake = serializar_ope("Hola", HANDSHAKE);
	//TODO log_debug(logFile, "Handshake a %s:%d", ip, puerto);
	conectar_cliente(&fd, puerto, ip);
	send(fd, handshake, strlen(handshake), 0);
	free(handshake);
	return fd;
}



char *recivirBytes(int socketE , int bytesCount, uint8_t * error){
	int allReceived = 0,tamanioPrefijo =0;
	int longNipc;
	char *bytesToReceive = malloc(sizeof(char)*10);

			int received= 0;

				received = recv(socketE , bytesToReceive , bytesCount - allReceived  , MSG_PEEK);
				if(received>5){
				tamanioPrefijo = obtener_tamanio_prefijo_payload(bytesToReceive);
				longNipc = obtener_tamanio_paquete(bytesToReceive)+ tamanioPrefijo;
				bytesToReceive = realloc(bytesToReceive,longNipc+1);

				received = recv(socketE , bytesToReceive , longNipc  , 0);

				bytesToReceive[longNipc]='\0';
				allReceived += received;
				*error=0;
				}
				else *error=1;

			return bytesToReceive;
}



int32_t levantar_server(int32_t *socket_s){

	int32_t yes = 1;
	struct sockaddr_in pp_addr;

	if ((*socket_s=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		perror("Error en creacion");
		return 1;
	}

	bzero((char *) &pp_addr, sizeof(pp_addr));
	pp_addr.sin_family = AF_INET;
	pp_addr.sin_addr.s_addr = INADDR_ANY;
	pp_addr.sin_port = htons(puertoHost);
	//len = sizeof(struct sockaddr_in);//todo

	if (setsockopt(*socket_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1) {
		perror("Error en setsock");
		return 1;
	}

	if (bind(*socket_s, (struct sockaddr *) &pp_addr, sizeof(struct sockaddr_in))==-1) {
		perror("Error en bind");
		return 1;
	}

	if (listen(*socket_s, MIN_CONEXIONES) ==  -1) {
		perror("Error en listen");
		return 1;
	}

	return 0;

}

int enviarScript(uint32_t fd_client,uint32_t fd_enserver, int16_t priority,char *codigo){
	t_file_package *paqArch = create_package_file( fd_enserver,priority,codigo);
	char * archSerial = serializar_arch(paqArch);
	send(fd_client,archSerial,strlen(archSerial),0);
	free(archSerial);
	free(codigo);


	free_package_file(paqArch);
return 0;
}

int enviar_variable (t_pcb *pcb,  char *var, int32_t valor){
	char * opeToSend;

	char * envi;
	asprintf(&envi,"Var:%s Valor:%d\n",var,valor);
	opeToSend = serializar_ope(envi,IMPRIMIR_VAR);
	send(pcb->socket_id, opeToSend, strlen(opeToSend), 0);
	free(envi);
	free(opeToSend);
	return 0;
}

