/*
 ============================================================================
 Name        : pi.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/temporal.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/types.h>
#include <unistd.h>

int socketClient (int argc, char **argv);

int main(int argc, char **argv) {

	socketClient (argc, argv);

	return EXIT_SUCCESS;
}
