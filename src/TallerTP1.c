/*
 ============================================================================
 Name        : TallerTP1.c
 Author      : Freddy
 Version     :
 Copyright   : Do not copy
 Description : TP1 Taller 1C2016
 ============================================================================
 */

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "socket.h"

#define BUFFER_LEN 1024
#define EXIT_BAD_INVOCATION 1
#define M 65536

int recv_message(int skt, char *buf, int size) {
	int received = 0;
	int s = 0;
	bool is_the_socket_valid = true;

	while (received < size && is_the_socket_valid) {
		s = recv(skt, &buf[received], size-received, MSG_NOSIGNAL);

		if (s == 0) { // nos cerraron el socket :(
			is_the_socket_valid = false;
		}
		else if (s < 0) { // hubo un error >(
			is_the_socket_valid = false;
		}
		else {
			received += s;
		}
	}

	if (is_the_socket_valid) {
		return received;
	}
	else {
		return -1;
	}
}

int send_message(int skt, char *buf, int size) {
	int sent = 0;
	int s = 0;
	bool is_the_socket_valid = true;

	while (sent < size && is_the_socket_valid) {
		s = send(skt, &buf[sent], size-sent, MSG_NOSIGNAL);

		if (s == 0) {
			is_the_socket_valid = false;
		}
		else if (s < 0) {
			is_the_socket_valid = false;
		}
		else {
			sent += s;
		}
	}

	if (is_the_socket_valid) {
		return sent;
	}
	else {
		return -1;
	}
}

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

int startServerAndWaitRequestNEW(char* port){
	/*printf("Me llamaron como server, apuntaria al puerto %s\n", port);

	char buffer[BUFFER_LEN];
	int s = 0;
	socket_t serverSkt;
	socket_t clientSkt;

	if (!socket_init(&serverSkt, port)) {
		exit(1);
	}

	//bind and listen
	if (!socket_bind_and_listen(&serverSkt, port)) {
		exit(2);
	}

	//accept
	if (!socket_accept(&serverSkt, &clientSkt)) {
		exit(3);
	}

	//receive
	memset(buffer, 0, BUFFER_LEN);
	socket_receive(&serverSkt, buffer, 10);

	s = strlen(buffer);
	printf("Se recibieron %i bytes\n", s);
	printf("El mensaje fue: %s\n", buffer);

	s = socket_shutdown(&clientSkt);
	s = socket_destroy(&clientSkt);
	s = socket_shutdown(&serverSkt);
	s = socket_destroy(&serverSkt);*/

	return 1;
}

int startServerAndWaitRequest(char* port){
	printf("Me llamaron como server, apuntaria al puerto %s\n", port);

	int s = 0;
	unsigned short len = 0;
	//	bool continue_running = true;
	//	bool is_the_accept_socket_valid = true;

	struct addrinfo hints;
	struct addrinfo *ptr;

	int skt, peerskt = 0;

	char buffer[BUFFER_LEN];
	//char *tmp;

	//socket_t serverSkt = socket();

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
	hints.ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
	hints.ai_flags = AI_PASSIVE;     /* AI_PASSIVE for server           */

	s = getaddrinfo(NULL, port, &hints, &ptr);
	if (s != 0) {
		printf("Error in getaddrinfo: %s\n", gai_strerror(s));
		exit(1);
	}

	skt = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (skt == -1) {
		printf("Error: %s\n", strerror(errno));
		freeaddrinfo(ptr);
		exit(1);
	}

	//s = socket_bind_and_listen(&serverSkt, port);
	//bind and listen
	s = bind(skt, ptr->ai_addr, ptr->ai_addrlen);
	if (s == -1) {
		printf("Error: %s\n", strerror(errno));
		close(skt);
		freeaddrinfo(ptr);
		exit(1);
	}

	freeaddrinfo(ptr);

	// Cuanto clientes podemos mantener en espera antes de poder acceptarlos?
	s = listen(skt, 20);
	if (s == -1) {
		printf("Error: %s\n", strerror(errno));
		close(skt);
		exit(1);
	}

	// s = socket_accept(&serverSkt, &clientSkt);
	peerskt = accept(skt, NULL, NULL);   // aceptamos un cliente
	if (peerskt == -1) {
		printf("Error: %s\n", strerror(errno));
		//		continue_running = false;
		//		is_the_accept_socket_valid = false;
	}else {
		printf("New client\n");
		memset(buffer, 0, BUFFER_LEN);
		recv_message(peerskt, buffer, BUFFER_LEN-1014);

		len = strlen(buffer);
		printf("Se recibieron %i bytes\n", len);
		printf("El mensaje fue: %s\n", buffer);

		// s = socket_shutdown(&clientSkt);
		// s = socket_destroy(&clientSkt);
		shutdown(peerskt, SHUT_RDWR);
		close(peerskt);
	}
	// s = socket_shutdown(&serverSkt);
	// s = socket_destroy(&serverSkt);
	shutdown(skt, SHUT_RDWR);
	close(skt);

	return EXIT_SUCCESS;
}

int requestFileFromServer(char* hostname, char* port, char* old_local_file, char* new_local_file, char* new_remote_file, char* block_size) {
	//Debug info
	printf("Me llamaron bien como cliente\n");
	printf("Hostname: %s\n", hostname);
	printf("Port: %s\n", port);
	printf("Old file: %s\n", old_local_file);
	printf("New file: %s\n", new_local_file);
	printf("Remote file: %s\n", new_remote_file);
	printf("Block size: %s\n", block_size);

	int blockSize = atoi(block_size);
	int i = 0;
	int checksum = 0;
	char buffer[blockSize];

	//abro file viejo
	FILE *oldLocalFile = fopen(old_local_file, "r");
	if (oldLocalFile == NULL) {
		return 1;
	}

	//abro file nuevo
	FILE *newLocalFile = fopen(new_local_file, "w");
	if (newLocalFile == NULL) {
		fclose(oldLocalFile);
		return 1;
	}

	//conecto socket
	//socket_t skt = socket();
	//socket_init(&skt);
	//socket_connect(&skt, hostname, port);


	fwrite("I - Mensaje - Checksum", sizeof(char), strlen("I - Mensaje - Checksum"), newLocalFile);
	//genero checksums y los mando
	while (fread(buffer, sizeof(char), blockSize, oldLocalFile) == blockSize) {
		printf("El buffer para procesar es: %s\n", buffer);
		checksum = generateChecksum(buffer);

		//lo mando con i como key
		fwrite((char*)&i, sizeof(int), 1, newLocalFile);
		fwrite(" - ", sizeof(char), strlen(" - "), newLocalFile);
		fwrite(buffer, sizeof(char), blockSize, newLocalFile);
		fwrite(" - ", sizeof(char), strlen(" - "), newLocalFile);
		fwrite((char*)&checksum, sizeof(int), 1, newLocalFile);
		fwrite("\n", sizeof(char), strlen("\n"), newLocalFile);

		i++;
	}

	//cierro socket y files
	//socket_shutdown(&skt);
	//socket_destroy(&skt);
	fclose(oldLocalFile);
	fclose(newLocalFile);

	return 0;
}

int main(int argc, char** argv) {
	puts("Welcome to TP1");

	//verifico si me llamaron bien en modo server
	if ((argc == 3) && (strncmp(argv[1], "server", 6) == 0)){
		return startServerAndWaitRequest(argv[2]);
	}
	//verifico si me llamaron bien en modo server
	if ((argc == 8) && (strncmp(argv[1], "client", 6) == 0)){
		return requestFileFromServer(argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
	}

	puts("Me llamaron mal, salgo al choto!!");
	return EXIT_BAD_INVOCATION;
}
