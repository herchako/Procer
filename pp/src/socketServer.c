#include <stdio.h>
#include <stdlib.h>
#include "commons/temporal.h"
#include <sys/socket.h>
#include "socket.h"
#include <commons/pcb.h>
#include <pthread.h>
#include <sys/types.h>

void *socketThread (void *);

int socketServer () {

	int8_t i;

//Creación del socket Server y espera de un cliente

	t_socket_server *server = sockets_createServer ("127.0.0.1",5301);

	t_socket_client *client;

	pthread_t chld_thr;

	sockets_listen (server);

	while (i < DEFAULT_MAX_CONEXIONS){

		client = sockets_accept (server);

		i++;

		if (client < 0) {

				fprintf (stderr, "Server:accept error\n");

				exit (0);
		}

		//Creación de un nuevo thread
		pthread_create (&chld_thr, NULL, (void *) &socketThread, (void *) client);

		pthread_detach (chld_thr);


	}


	sockets_destroyClient (client);

	sockets_destroyServer (server);

	return EXIT_SUCCESS;

}


void *socketThread (void *arg) {

	t_socket_client *newSocket = arg;

	t_socket_buffer *buffer = malloc( sizeof(t_socket_buffer) );

	printf ("Número de Thread= [%d]: Número de Socket=%d\n", (int)pthread_self(), (int)newSocket);

	//Recibir el archivo

		int32_t flag=0, recibidos;

		while(flag == 0){

			recibidos = sockets_recvInBuffer(newSocket, buffer);

			printf ("%s\n", buffer->data);

			if (buffer->data[recibidos]== '\0')
				flag=1;

				//Guardado de la estructura en el PCB
		}

		printf ("Terminó el archivo \n");

	//

	sockets_bufferDestroy (buffer);

	sockets_destroyClient (newSocket);

	pthread_exit ((void *)0);

}
