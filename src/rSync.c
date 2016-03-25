/*
 * rSync.c
 *
 *  Created on: Mar 24, 2016
 *      Author: freddy
 */

#ifndef SOCKET_C
#define SOCKET_C

#include "socket.h"

#define M 65536
#define MAX_SMALL_BUF_LEN 5 //4 bytes

#define CODE_CHECKSUM "1"
#define CODE_END_OF_CHECKSUM_LIST "2"

#define RSYNC_NO_ERROR 0
#define RSYNC_CLIENT_ERROR 1
#define RSYNC_SERVER_ERROR 1

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

int requestFileFromServer(char* hostname, char* port, char* old_local_file, char* new_local_file, char* new_remote_file, char* block_size) {
	// TODO Sacar estos prints, el cliente no imprime nada
	printf("Client parameters\n");
	printf("Hostname: %s\n", hostname);
	printf("Port: %s\n", port);
	printf("Old file: %s\n", old_local_file);
	printf("New file: %s\n", new_local_file);
	printf("Remote file: %s\n", new_remote_file);
	printf("Block size: %s\n", block_size);

	int aux, len, blockSize, checksum;
	char buffer[MAX_SMALL_BUF_LEN];
	char* fileBuffer;
	socket_t skt;

	printf("Opening file to update: %s\n", old_local_file);
	FILE *oldLocalFile = fopen(old_local_file, "r");
	if (oldLocalFile == NULL) {
		return RSYNC_CLIENT_ERROR;
	}

	printf("Creating updated file: %s\n", new_local_file);
	FILE *newLocalFile = fopen(new_local_file, "w");
	if (newLocalFile == NULL) {
		fclose(oldLocalFile);
		return RSYNC_CLIENT_ERROR;
	}

	blockSize = atoi(block_size);

	printf("Conecting to %s on port: %s\n", hostname, port);
	aux = socket_init_client(&skt, port, hostname);
	if (aux != 0) {
		fclose(newLocalFile);
		fclose(oldLocalFile);
		return RSYNC_CLIENT_ERROR;
	}	//pasar esto lo de abajo
	//if (ESTO =! 0) return closeClientAndReturnError(newLocalFile, oldLocalFile, &skt);

	printf("Asking for file: %s\n", new_remote_file);
	len = strlen(new_remote_file);
	snprintf(buffer, sizeof(buffer), "%04d", len);
	aux = socket_send(&skt,buffer,sizeof(int));
	aux = socket_send(&skt, new_remote_file, len);

	printf("Sending blocksize\n");
	snprintf(buffer, sizeof(buffer), "%04d", blockSize);
	aux = socket_send(&skt,buffer,sizeof(int));

	printf("Sending checksums\n");
	/*
	 * Para mandar los checksums primero mando un byte con ​0x01
	 * seguido del checksum en 4 bytes del blocksize, asi por cada
	 * checksum que genero. Descarto el ultimo bloque si no llego
	 * a leer un blocksize completo (ver si necesito el cachito este)
	 * Finalmente mando un fin de lista de checksums mandando
	 * un 0x02. Despues de esto paso a leer y construir el file
	 * */
	fileBuffer = (char*) calloc(blockSize+1,sizeof(char));
	while (fread(fileBuffer, sizeof(char), blockSize, oldLocalFile) == blockSize) {
		aux = socket_send(&skt,CODE_CHECKSUM,sizeof(char));
		checksum = generateChecksum(fileBuffer);
		snprintf(buffer, sizeof(buffer), "%04d", checksum);
		aux = socket_send(&skt,buffer,4*sizeof(char));
	}
	aux = socket_send(&skt,CODE_END_OF_CHECKSUM_LIST,sizeof(char));

	printf("Recreating file\n");
	/*
	 *
	 * */

	printf("Closing and destroying everything\n");
	aux = socket_destroy(&skt);
	fclose(newLocalFile);
	fclose(oldLocalFile);

	return RSYNC_NO_ERROR;

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
}

int startServerAndWaitRequestForFile(char* port){
	int aux;
	char buffer[MAX_SMALL_BUF_LEN];
	socket_t server;
	socket_t client;
	char* tmp;

	printf("Starting server on port: %s\n", port);
	aux = socket_init_server(&server, port);
	if (aux == -1) return -1;

	printf("Listening for a conection\n");
	aux = socket_listen(&server, 1);
	if (aux == -1) {
		socket_destroy(&server);
		return -1;
	}

	printf("Waiting connection\n");
	aux = socket_accept(&server, &client);
	if (aux == -1) {
		socket_destroy(&server);
		return -1;
	}
	printf("Client connected\n");

	printf("Getting file request\n");
	memset(buffer, 0, MAX_SMALL_BUF_LEN);
	aux = socket_receive(&client, buffer, MAX_SMALL_BUF_LEN-1);
	//fallo aca me las tomo
	aux = atoi(buffer);
	tmp = (char*) calloc(aux,sizeof(char));
	socket_receive(&client, tmp, aux);
	//fallo aca tambien me las tomo
	printf("File: %s\n", tmp);
	FILE *oldLocalFile = fopen(tmp, "r");
	if (tmp == NULL) {
		return 1;
	}

	aux = socket_send(&client, tmp, aux);


	fclose(oldLocalFile);
	aux = socket_destroy(&client);
	aux = socket_destroy(&server);

	return RSYNC_NO_ERROR;
}

#endif /* RSYNC_C_ */
