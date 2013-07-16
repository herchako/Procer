#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <commons/temporal.h>
#include "socket.h"
#include <stdint.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/types.h>
#include <unistd.h>

int socketClient (int argc, char **argv) {

	//Lee del archivo de configuración la IP y el Puerto

		printf ("Corriendo Cliente");
		t_config *arch_config = config_create("/home/utnso/git/2012-2c-medrano-y-labashe/procer/procerConfigs/pi.cfg");

		const char *IPHost = config_get_string_value(arch_config, "pp.host");

		uint32_t puertoHost = config_get_int_value(arch_config, "pp.puerto");

		printf("Ip Host: %s", IPHost);

		printf("Puerto Host: %d", puertoHost);


	//Creación del Socket y conexión con el Servidor

		t_socket_client *client = sockets_createClient ((char*)IPHost,puertoHost);

		sockets_connect (client,"127.0.0.1",5301);

		t_socket_buffer *buffer = malloc( sizeof(t_socket_buffer) );


	//Lectura del archivo a enviar

		long longList,falta, tam_buffer;
		int32_t i, enviados, leidos;

		FILE *flista ;

		FILE *fd = fopen((const char * __restrict__)argv[1], "rb" );

		fseek( fd , 0 , SEEK_END ); // Se posiciona al final del archivo

		int tamanio = ftell(fd); // Devuelve el tamaño del archivo (en bytes)

		fclose(fd);

		printf ("tamaño %i\n", tamanio);

		longList = htonl(tamanio); // longitud del archivo

		longList= tamanio; //tamaño del archivo

		tam_buffer=1024;

		flista= fopen ((const char * __restrict__)argv[1], "rb");

		if (flista!=NULL){

			falta = longList;

			i = 0;

			while(i < longList){

				if (falta < tam_buffer) {  //acá entra si el archivo es menor que 1024 o si ya tiene más de un envio

					tam_buffer=falta + 1;

					buffer->data[falta]='\0';

				}
				printf("tam_buffer: %ld - falta: %ld\n", tam_buffer, falta);

				leidos = fread(buffer->data, sizeof(char),tam_buffer,flista);

				buffer->size= sizeof (buffer->data);

				enviados=sockets_sendBuffer(client, buffer);

				printf("Envie %d bytes \n", enviados);

				falta = falta-leidos;

				i = i + leidos;
			}
		}

			else printf("No abrio el archivo \n");


	//Conexión por socket

		sockets_destroyClient (client);

		return EXIT_SUCCESS;



}
