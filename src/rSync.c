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
		if (lista_iter_ver_actual(iterador) != NULL &&
				(*((int*)lista_iter_ver_actual(iterador)) == checksum))
			checksumEncontrado = i;
	} while ((lista_iter_avanzar(iterador) == 0) && (checksumEncontrado == 0));

	lista_iter_destruir(iterador);
	return checksumEncontrado;
}

int requestFileFromServer(char* hostname, char* port, char* old_local_file,
		char* new_local_file, char* new_remote_file, char* block_size) {
	int aux, len, blockSize, checksum, codeRecieved, fileBufferSize;
	bool endOfUpdate;
	char intBuffer[MAX_SMALL_BUF_LEN];
	char flagBuffer;
	char* fileBuffer;
	socket_t skt;

	memset(intBuffer, 0, MAX_SMALL_BUF_LEN);

	// abro file a actualizar
	FILE *oldLocalFile = fopen(old_local_file, "r");
	if (oldLocalFile == NULL) {
		return RSYNC_CLIENT_ERROR;
	}

	// inicio file actualizado
	FILE *newLocalFile = fopen(new_local_file, "w");
	if (newLocalFile == NULL) {
		fclose(oldLocalFile);
		return RSYNC_CLIENT_ERROR;
	}

	blockSize = atoi(block_size);

	// me conecto al servidor
	aux = socket_init_client(&skt, port, hostname);
	if (aux != 0) {
		fclose(newLocalFile);
		fclose(oldLocalFile);
		return RSYNC_CLIENT_ERROR;
	}

	// envio largo nombre y nombre file actualizado
	len = strlen(new_remote_file);
	snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, len);
	aux = socket_send(&skt,intBuffer,4*sizeof(char));
	aux = socket_send(&skt, new_remote_file, len);

	// envio blocksize
	snprintf(intBuffer, sizeof(intBuffer), FORMATO_NUMERICO_RSYNC, blockSize);
	aux = socket_send(&skt,intBuffer,sizeof(int));

	// envio checksums
	fileBuffer = (char*) calloc(blockSize+1,sizeof(char));
	while (fread(fileBuffer, sizeof(char), blockSize, oldLocalFile) == blockSize) {
		aux = socket_send(&skt,CODE_CHECKSUM,sizeof(char));
		checksum = generateChecksum(fileBuffer);
		checksum = htonl(checksum);
		aux = socket_send(&skt,(char*)&checksum,4*sizeof(char));
	}
	aux = socket_send(&skt,CODE_END_OF_CHECKSUM_LIST,sizeof(char));

	// recreo el archivo
	free(fileBuffer);
	fileBufferSize = 2*blockSize;
	fileBuffer = (char*) calloc(fileBufferSize,sizeof(char));
	endOfUpdate = false;
	while (!endOfUpdate) {
		aux = socket_receive(&skt, &flagBuffer, sizeof(char));
		codeRecieved = flagBuffer - '0';
		switch (codeRecieved) {
		case CODE_INT_FOUND_CHECKSUM:
			aux = socket_receive(&skt,intBuffer,4*sizeof(char));
			len = blockSize;
			checksum = atoi(intBuffer);
			fseek(oldLocalFile, checksum * blockSize, SEEK_SET);
			aux = fread(fileBuffer, sizeof(char), blockSize, oldLocalFile);
			printf("RECV Block index %i\n", checksum);
			break;
		case CODE_INT_NEW_BYTES:
			aux = socket_receive(&skt,intBuffer,4*sizeof(char));
			len = atoi(intBuffer);
			if (len >= fileBufferSize){
				fileBufferSize = len+1;
				char* auxString = calloc(fileBufferSize,sizeof(char));
				free(fileBuffer);
				fileBuffer = auxString;
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

	// cierro y libero recursos
	free(fileBuffer);
	aux = socket_shutdown(&skt);
	aux = socket_destroy(&skt);
	fclose(newLocalFile);
	fclose(oldLocalFile);

	return RSYNC_NO_ERROR;
}

int startServerAndWaitRequestForFile(char* port){
	int i, j, aux, fileNameLen, blockSize, checksum, fileNewDataLen;
	int checksumEncontrado;
	char intBuffer[MAX_SMALL_BUF_LEN];
	char byteBuffer = EOF;
	char letter = EOF;
	char* fileBuffer;
	char* fileName;
	char* fileNewData;
	socket_t server;
	socket_t client;
	lista_t* lista;

	//inicio server en puerto
	aux = socket_init_server(&server, port);
	if (aux != 0) return RSYNC_SERVER_ERROR;

	//espero cliente
	aux = socket_listen(&server, 1);
	if (aux != 0) {
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}

	//acepto cliente
	aux = socket_accept(&server, &client);
	if (aux != 0) {
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}

	//consigo largo y nombre del archivo actualizado
	memset(intBuffer, 0, MAX_SMALL_BUF_LEN);
	aux = socket_receive(&client, intBuffer, MAX_SMALL_BUF_LEN-1);
	if (aux != 0) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	fileNameLen = atoi(intBuffer);
	fileName = (char*) calloc(fileNameLen+1,sizeof(char));
	aux = socket_receive(&client, fileName, fileNameLen);
	if (aux != 0) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	//abro file actualizado
	FILE *localFile = fopen(fileName, "r");
	if (localFile == NULL) {
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	//leo blocksize
	aux = socket_receive(&client, intBuffer, MAX_SMALL_BUF_LEN-1);
	if (aux != 0) {
		fclose(localFile);
		socket_destroy(&client);
		socket_destroy(&server);
		return RSYNC_SERVER_ERROR;
	}
	blockSize = atoi(intBuffer);

	//recibo y guardo checksums cliente
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

	//empiezo a crear y comparar checksums locales
	checksumEncontrado = 1;
	fileNewData = (char*) calloc(30,sizeof(char));
	fileBuffer = (char*) calloc(blockSize+1,sizeof(char));
	j = 0;
	while (true) {
		i = 0;
		//avanzamos ventana totalmente o solo un caracter
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
		if ((checksumEncontrado != 0)) {
			//enviamos lo nuevo si hace falta y el checksum encontrado
			checksumEncontrado--; //fix porque numeran desde cero
			fileNewDataLen = strlen(fileNewData);
			if (fileNewDataLen != 0) {
				aux = socket_send(&client, CODE_NEW_BYTES, 1);
				snprintf(intBuffer, sizeof(intBuffer),
						FORMATO_NUMERICO_RSYNC, fileNewDataLen);
				aux = socket_send(&client, intBuffer, 4*sizeof(char));
				aux = socket_send(&client, fileNewData, fileNewDataLen);
				memset(fileNewData,0,strlen(fileNewData));
				j = 0;
			}
			aux = socket_send(&client, CODE_FOUND_CHECKSUM, 1);
			snprintf(intBuffer, sizeof(intBuffer),
					FORMATO_NUMERICO_RSYNC, checksumEncontrado);
			aux = socket_send(&client, intBuffer, 4*sizeof(char));
			checksumEncontrado++; //deshago fix
		} else {
			//checksum no encontrado, agrego a nuevos datos
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
	fileNewDataLen = 0;
	while (fileNewData[fileNewDataLen] != EOF &&
			fileNewData[fileNewDataLen] != '\0')
		fileNewDataLen++;
	if (fileNewDataLen != 0) {
		aux = socket_send(&client, CODE_NEW_BYTES, 1);
		snprintf(intBuffer, sizeof(intBuffer),
				FORMATO_NUMERICO_RSYNC, fileNewDataLen);
		aux = socket_send(&client, intBuffer, 4*sizeof(char));
		aux = socket_send(&client, fileNewData, fileNewDataLen);
	}
	aux = socket_send(&client, CODE_EOF, 1);

	//destruyo y libero toda la memoria utilizada
	while (!lista_esta_vacia(lista)){
		free((int*)lista_borrar_primero(lista));
	}
	lista_destruir(lista);
	free(fileNewData);
	free(fileBuffer);
	free(fileName);
	fclose(localFile);
	aux = socket_shutdown(&client);
	aux = socket_destroy(&client);
	aux = socket_shutdown(&server);
	aux = socket_destroy(&server);

	return RSYNC_NO_ERROR;
}

#endif /* RSYNC_C_ */
