/*
 * rSync.c
 *
 *  Created on: Mar 24, 2016
 *      Author: freddy
 */

#ifndef SOCKET_C
#define SOCKET_C

#include "socket.h"
#include "lista.h"

#define M 65536 //pow(2,16)
#define MAX_SMALL_BUF_LEN 5 //4 bytes + EOL

#define CODE_CHECKSUM "1"
#define CODE_END_OF_CHECKSUM_LIST "2"
#define CODE_NEW_BYTES "3"
#define CODE_INT_NEW_BYTES 3
#define CODE_FOUND_CHECKSUM "4"
#define CODE_INT_FOUND_CHECKSUM 4
#define CODE_EOF "5"
#define CODE_INT_EOF 5

#define RSYNC_NO_ERROR 0
#define RSYNC_CLIENT_ERROR 1
#define RSYNC_SERVER_ERROR 1

#define FORMATO_NUMERICO_RSYNC "%04d"

int generateChecksum(char* buffer) {
	int lower = 0;
	int higher = 0;
	int i;
	int largoBuffer = strlen(buffer);

	for (i = 0; i < largoBuffer; i++) {
		lower += buffer[i];
		higher += (largoBuffer - i) * buffer[i];
	}
	lower = lower % M;
	higher = higher % M;

	return (lower + higher * M);
}

int lista_tiene_checksum(lista_t *lista, int checksum) {
	int checksumEncontrado = 0;
	int i = 0;
	lista_iter_t* iterador = lista_iter_crear(lista);

	do {
		i++;
		if (lista_iter_ver_actual(iterador) != NULL && (*((int*)lista_iter_ver_actual(iterador)) == checksum))
			checksumEncontrado = i;
	} while ((lista_iter_avanzar(iterador) == 0) && (checksumEncontrado == 0));

	lista_iter_destruir(iterador);
	return checksumEncontrado;
}

int requestFileFromServer(char* hostname, char* port, char* old_local_file, char* new_local_file, char* new_remote_file, char* block_size) {
	printf("Client parameters\n");
	printf("Hostname: %s\n", hostname);
	printf("Port: %s\n", port);
	printf("Old file: %s\n", old_local_file);
	printf("New file: %s\n", new_local_file);
	printf("Remote file: %s\n", new_remote_file);
	printf("Block size: %s\n", block_size);

	int aux, len, blockSize, checksum, codeRecieved;
	bool endOfUpdate;
	char intBuffer[MAX_SMALL_BUF_LEN];
	char flagBuffer;
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

	printf("ConNecting to %s on port: %s\n", hostname, port);
	aux = socket_init_client(&skt, port, hostname);
	if (aux != 0) {
		fclose(newLocalFile);
		fclose(oldLocalFile);
		return RSYNC_CLIENT_ERROR;
	}

	printf("Asking for file: %s\n", new_remote_file);
	len = strlen(new_remote_file);
	snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, len);
	aux = socket_send(&skt,intBuffer,4*sizeof(char));
	aux = socket_send(&skt, new_remote_file, len);

	printf("Sending blocksize\n");
	snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, blockSize);
	aux = socket_send(&skt,intBuffer,sizeof(int));

	printf("Sending checksums\n");
	fileBuffer = (char*) calloc(blockSize+1,sizeof(char));
	while (fread(fileBuffer, sizeof(char), blockSize, oldLocalFile) == blockSize) {
		aux = socket_send(&skt,CODE_CHECKSUM,sizeof(char));
		checksum = generateChecksum(fileBuffer);
		aux = socket_send(&skt,(char*)&checksum,4*sizeof(char));
	}
	aux = socket_send(&skt,CODE_END_OF_CHECKSUM_LIST,sizeof(char));

	printf("Recreating file\n");
	free(fileBuffer);
	fileBuffer = (char*) calloc(blockSize*2,sizeof(char));
	endOfUpdate = false;
	while (!endOfUpdate) {
		aux = socket_receive(&skt, &flagBuffer, sizeof(char));
		codeRecieved = atoi(&flagBuffer);
		switch (codeRecieved) {
		case CODE_INT_FOUND_CHECKSUM:
			aux = socket_receive(&skt,intBuffer,4*sizeof(char));
			len = blockSize;
			checksum = atoi(intBuffer);
			fseek(oldLocalFile, (checksum - 1) * blockSize, SEEK_SET);
			fread(fileBuffer, sizeof(char), blockSize, oldLocalFile);
			printf("RECV Block index %i\n", checksum);
			break;
		case CODE_INT_NEW_BYTES:
			aux = socket_receive(&skt,intBuffer,4*sizeof(char));
			len = atoi(intBuffer);
			if (len >= strlen(fileBuffer)){
				char* auxString = calloc(strlen(fileBuffer)*2,sizeof(char));
				memcpy(auxString, fileBuffer, strlen(fileBuffer));
				char* swap = fileBuffer;
				fileBuffer = auxString;
				free(swap);
			}
			aux = socket_receive(&skt,fileBuffer,len);
			printf("RECV File chunk %i bytes\n", len);
			break;
		case CODE_INT_EOF:
			endOfUpdate = true;
			printf("RECV End of file\n");
			break;
		}
		if (!endOfUpdate) fwrite(fileBuffer, sizeof(char), len, newLocalFile);
		memset(fileBuffer, 0, strlen(fileBuffer));
	}
	printf("File recreated\n");

	printf("Closing and destroying everything\n");
	free(fileBuffer);
	aux = socket_destroy(&skt);
	fclose(newLocalFile);
	fclose(oldLocalFile);

	printf("Exiting\n");
	return RSYNC_NO_ERROR;
}

int startServerAndWaitRequestForFile(char* port){
	printf("Server parameters\n");
	printf("Port: %s\n", port);
	int i, j, aux, fileNameLen, blockSize, checksum, fileNewDataLen;
	int checksumEncontrado;
	char intBuffer[MAX_SMALL_BUF_LEN];
	char byteBuffer, letter;
	char* fileBuffer;
	char* fileName;
	char* fileNewData;
	socket_t server;
	socket_t client;
	lista_t* lista;

	printf("Starting server on port: %s\n", port);
	aux = socket_init_server(&server, port);
	if (aux != 0) return RSYNC_SERVER_ERROR;

	printf("Listening for a connection\n");
	aux = socket_listen(&server, 1);
	if (aux != 0) {
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}

	printf("Accepting next connection...\n");
	aux = socket_accept(&server, &client);
	if (aux != 0) {
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	printf("Client connected\n");

	printf("Getting filename length\n");
	memset(intBuffer, 0, MAX_SMALL_BUF_LEN);
	aux = socket_receive(&client, intBuffer, MAX_SMALL_BUF_LEN-1);
	if (aux != 0) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	fileNameLen = atoi(intBuffer);
	printf("Filename Length received: %i\n",fileNameLen);
	printf("Getting filename\n");
	fileName = (char*) calloc(fileNameLen+1,sizeof(char));
	aux = socket_receive(&client, fileName, fileNameLen);
	if (aux != 0) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	printf("Opening file: %s\n", fileName);
	FILE *localFile = fopen(fileName, "r");
	if (localFile == NULL) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	printf("File opened\n");

	printf("Getting block size\n");
	aux = socket_receive(&client, intBuffer, MAX_SMALL_BUF_LEN-1);
	if (aux != 0) {
		fclose(localFile);
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	blockSize = atoi(intBuffer);
	printf("Block size received: %i\n",blockSize);

	printf("Storing checksums received in list\n");
	lista = lista_crear();
	while ((aux = socket_receive(&client, &byteBuffer, sizeof(char)))==0) {
		if (byteBuffer=='1') {
			char* cksum = calloc(4, sizeof(char));
			aux = socket_receive(&client, cksum, MAX_SMALL_BUF_LEN-1);
			int* checksum = (int*)cksum;
			*checksum = ntohl(*checksum);
			lista_insertar_ultimo(lista,checksum);
		} else if (byteBuffer=='2') {
			break;
		} else {
			return RSYNC_SERVER_ERROR;
		}
	}
	if (aux != 0) {
		fclose(localFile);
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	printf("List of checksums complete\n");

	printf("Generating and looking for checksums\n");
	checksumEncontrado = 1;
	fileNewData = (char*) calloc(30,sizeof(char));
	fileBuffer = (char*) calloc(blockSize+1,sizeof(char));
	j = 0;
	while (true) {
		i = 0;
		//avanzamos ventana
		if (checksumEncontrado != 0) {
			while (i < blockSize) {
				letter = fgetc(localFile);
				fileBuffer[i] = letter;
				i++;
			}
		} else {
			while (i < blockSize) {
				fileBuffer[i] = fileBuffer[i+1];
				i++;
			}
			letter = fgetc(localFile);
			fileBuffer[blockSize-1] = letter;
		}
		//generamos su checksum y lo buscamos
		checksum = generateChecksum(fileBuffer);
		checksumEncontrado = lista_tiene_checksum(lista, checksum);
		//enviamos lo nuevo si hace falta y el checksum encontrado
		if ((checksumEncontrado != 0)) {
			fileNewDataLen = strlen(fileNewData);
			if (fileNewDataLen != 0) {
				aux = socket_send(&client, CODE_NEW_BYTES, 1);
				snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, fileNewDataLen);
				aux = socket_send(&client, intBuffer, 4*sizeof(char));
				aux = socket_send(&client, fileNewData, fileNewDataLen);
				memset(fileNewData,0,strlen(fileNewData));
				j = 0;
			}
			aux = socket_send(&client, CODE_FOUND_CHECKSUM, 1);
			snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, checksumEncontrado);
			aux = socket_send(&client, intBuffer, 4*sizeof(char));
		} else { //checksum no encontrado, agrego a nuevos datos
			fileNewData[j] = fileBuffer[0];
			j++;
		}
		if (letter == EOF) break;
	}
	//envio el remanente
	i = 1;
	while (fileBuffer[i] != EOF) {
		fileNewData[j] = fileBuffer[i];
		i++; j++;
	}
	fileNewDataLen = strlen(fileNewData);
	if (fileNewDataLen != 0) {
		aux = socket_send(&client, CODE_NEW_BYTES, 1);
		snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, fileNewDataLen);
		aux = socket_send(&client, intBuffer, 4*sizeof(char));
		aux = socket_send(&client, fileNewData, fileNewDataLen);
	}
	aux = socket_send(&client, CODE_EOF, 1);
	printf("Sent checksums found and new parts\n");

	printf("Releasing resources\n");
	//destruyo y libero toda la memoria utilizada
	while (!lista_esta_vacia(lista)){
		free((int*)lista_borrar_primero(lista));
	}
	lista_destruir(lista);
	free(fileName);
	fclose(localFile);
	aux = socket_destroy(&client);
	aux = socket_destroy(&server);

	printf("Exiting\n");
	return RSYNC_NO_ERROR;
}

#endif /* RSYNC_C_ */
