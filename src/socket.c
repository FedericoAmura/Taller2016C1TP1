#ifndef SOCKET_C
#define SOCKET_C
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

#define SOCKET_NO_ERROR 0
#define SOCKET_ERROR_CREANDO 1
#define SOCKET_ERROR_DESTRUYENDO 2
#define SOCKET_ERROR_BIND 3
#define SOCKET_ERROR_LISTEN 4
#define SOCKET_ERROR_SEND 5
#define SOCKET_ERROR_RECEIVE 6
#define SOCKET_ERROR_SHUTDOWN 7
#define SOCKET_ERROR_ACCEPT 8

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* Se trata de un socket simplificado */

struct socket {
	int socketfd;
//	struct addrinfo hints;
//	struct addrinfo *ptr;
};

/* ******************************************************************
 *                    PRIMITIVAS DEL SOCKET
 * *****************************************************************/

int socket_init(socket_t* this, char* port){
//	int aux;
//	memset(&(this->hints), 0, sizeof(struct addrinfo));
//	(this->hints).ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
//	(this->hints).ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
//	(this->hints).ai_flags = AI_PASSIVE;     /* AI_PASSIVE for server           */
//
//	aux = getaddrinfo(NULL, port, &(this->hints), &(this->ptr));
//	if (aux != 0) {
//		printf("Error in getaddrinfo: %s\n", gai_strerror(aux));
//		return SOCKET_ERROR_CREANDO;
//	}
//
//	aux = socket(this->ptr->ai_family, this->ptr->ai_socktype, this->ptr->ai_protocol);
//	if (aux == -1) {
//		printf("Error: %s\n", strerror(errno));
//		freeaddrinfo(this->ptr);
//		return SOCKET_ERROR_CREANDO;
//	}
//
//	this->socketfd = aux;
	int aux = socket(AF_INET, SOCK_STREAM, 0);

	if (aux == -1) return SOCKET_ERROR_CREANDO;

	this->socketfd = aux;

	return SOCKET_NO_ERROR;
}

int socket_destroy(socket_t* this){
//	free(this->ptr);
//	close(this->socketfd);
	return SOCKET_NO_ERROR;
}

int socket_bind_and_listen(socket_t* this, char* port){
//	int aux;
//	//bind
//	aux = bind(this->socketfd, this->ptr->ai_addr, this->ptr->ai_addrlen);
//	if (aux == -1) return SOCKET_ERROR_BIND;
//	freeaddrinfo(this->ptr);
//
//	//listen
//	aux = listen(this->socketfd, 10);
//	if (aux == -1) return SOCKET_ERROR_LISTEN;
//
	return SOCKET_NO_ERROR;
}

int socket_send(socket_t* this, char* buffer, unsigned int size){
//	int sent = 0;
//	int s = 0;
//	bool is_the_socket_valid = true;
//
//	while (sent < size && is_the_socket_valid) {
//		s = send(this->socketfd, &buffer[sent], size-sent, MSG_NOSIGNAL);
//
//		if (s == 0) {
//			is_the_socket_valid = false;
//		}
//		else if (s < 0) {
//			is_the_socket_valid = false;
//		}
//		else {
//			sent += s;
//		}
//	}
//
//	if (is_the_socket_valid) {
//		return sent;
//	}
//	else {
//		return SOCKET_ERROR_SEND;
//	}
//
	return SOCKET_NO_ERROR;
}

int socket_receive(socket_t* this, char* buffer, unsigned int size){
//	int received = 0;
//	int s = 0;
//	bool is_the_socket_valid = true;
//
//	while (received < size && is_the_socket_valid) {
//		s = recv(this->socketfd, &buffer[received], size-received, MSG_NOSIGNAL);
//
//		if (s == 0) { //socket cerrado
//			is_the_socket_valid = false;
//		}
//		else if (s < 0) { //error en el socket
//			is_the_socket_valid = false;
//		}
//		else {
//			received += s;
//		}
//	}
//
//	if (is_the_socket_valid) {
//		return received;
//	}
//	else {
//		return SOCKET_ERROR_RECEIVE;
//	}
//
	return SOCKET_NO_ERROR;
}

int socket_shutdown(socket_t* this){
//	shutdown(this->socketfd,SHUT_RDWR);
	return SOCKET_NO_ERROR;
}

int socket_accept(socket_t* this, socket_t* cliente){
//	int aux;
//	aux = accept(this->socketfd, NULL, NULL);
//	if (aux == -1) return SOCKET_ERROR_ACCEPT;
//
//	cliente->socketfd = aux;
	return SOCKET_NO_ERROR;
}

#endif // SOCKET_C
