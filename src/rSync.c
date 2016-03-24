/*
 * rSync.c
 *
 *  Created on: Mar 24, 2016
 *      Author: freddy
 */

#ifndef SOCKET_C
#define SOCKET_C

#include <stdio.h>
#include <string.h>

#define M 65536
#include "socket.h"

int generateChecksum(char* buffer) {
	int lower = 0;
	int higher = 0;
	int i;
	int largoBuffer = strlen(buffer);

	for (i = 0; i < largoBuffer; i++) {
		lower += buffer[i];
		higher += buffer[largoBuffer-i];
	}
	lower = lower % M;
	higher = higher % M;

	return (lower + higher * M);
}

//int requestFileFromServer(char* hostname, char* port, char* old_local_file, char* new_local_file, char* new_remote_file, char* block_size) {
//	//Debug info
//	printf("Me llamaron bien como cliente\n");
//	printf("Hostname: %s\n", hostname);
//	printf("Port: %s\n", port);
//	printf("Old file: %s\n", old_local_file);
//	printf("New file: %s\n", new_local_file);
//	printf("Remote file: %s\n", new_remote_file);
//	printf("Block size: %s\n", block_size);
//
//	int aux;
//	socket_t skt;
//
//	aux = socket_init_client(&skt, port, hostname);
//	aux = socket_send(&skt, "Vamos los pibeeee!!!", 20);
//	aux = socket_destroy(&skt);
//
//	return 0;
//
//	//int aux;
//	int blockSize = atoi(block_size);
//	int i = 0;
//	int checksum = 0;
//	char buffer[blockSize];
//	//socket_t skt;
//
//	//abro file viejo
//	FILE *oldLocalFile = fopen(old_local_file, "r");
//	if (oldLocalFile == NULL) {
//		return 1;
//	}
//
//	//abro file nuevo
//	FILE *newLocalFile = fopen(new_local_file, "w");
//	if (newLocalFile == NULL) {
//		fclose(oldLocalFile);
//		return 1;
//	}
//
//	//conecto socket
//	//aux = socket_init_client(&skt);
//	aux = socket_connect(&skt,"127.0.0.1","4563"); //no me esta aceptando la conexion
//
//	//generacion y envio de checksums al server
//	fwrite("I - Mensaje - Checksum", sizeof(char), strlen("I - Mensaje - Checksum"), newLocalFile);
//	//genero checksums y los mando
//	while (fread(buffer, sizeof(char), blockSize, oldLocalFile) == blockSize) {
//		printf("El buffer para procesar es: %s\n", buffer);
//		checksum = generateChecksum(buffer);
//
//		//lo mando con i como key
//		fwrite((char*)&i, sizeof(int), 1, newLocalFile);
//		fwrite(" - ", sizeof(char), strlen(" - "), newLocalFile);
//		fwrite(buffer, sizeof(char), blockSize, newLocalFile);
//		fwrite(" - ", sizeof(char), strlen(" - "), newLocalFile);
//		fwrite((char*)&checksum, sizeof(int), 1, newLocalFile);
//		fwrite("\n", sizeof(char), strlen("\n"), newLocalFile);
//
//		i++;
//	}
//
//	//etapa de recibir vistazos de checksums y literales
//	aux = socket_receive(&skt, buffer, 50);
//
//	//cierro socket y files
//	aux = socket_destroy(&skt);
//	fclose(oldLocalFile);
//	fclose(newLocalFile);
//
//	return 0;
//}

int startServerAndWaitRequestForFile(char* port){
	printf("Me llamaron como server, apuntaria al puerto %s\n", port);

	int aux;
	char buffer[sizeof(int)];
	socket_t server;
	socket_t client;

	aux = socket_init_server(&server, port);
	if (aux == -1) return -1;

	aux = socket_listen(&server, 1);
	if (aux == -1) {
		socket_destroy(&server);
		return -1;
	}

	aux = socket_accept(&server, &client);
	if (aux == -1) {
		socket_destroy(&server);
		return -1;
	}

	aux = socket_send(&client, "Todos putooooos!!!!", 19);

	aux = socket_destroy(&client);
	aux = socket_destroy(&server);

	return 0;
}

int requestFileFromServer(char* port) {
	printf("Me llamaron como server, apuntaria al puerto %s\n", port);

	int aux;
	char buffer[sizeof(int)];
	socket_t server;
	socket_t client;

	aux = socket_init_server(&server, port);
	aux = socket_listen(&server, 1);
	aux = socket_accept(&server, &client);

	aux = socket_send(&client, "Todos putooooos!!!!", 19);

	aux = socket_destroy(&client);
	aux = socket_destroy(&server);

	return 0;
}

#endif /* RSYNC_C_ */
