#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 1
#endif

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
#define SOCKET_ERROR_CONECTANDO 9

/* ******************************************************************
 *                    PRIMITIVAS DEL SOCKET
 * *****************************************************************/

int socket_init(socket_t* this){
	int aux = socket(AF_INET, SOCK_STREAM, 0);
	if (aux==-1) {
		return SOCKET_ERROR_CREANDO;
	}

	this->socketfd = aux;
	return 0;
}

int socket_destroy(socket_t* this){
	//	free(this->ptr);
	//	close(this->socketfd);
	int aux = close(this->socketfd);
	if (aux != 0) {
		return SOCKET_ERROR_DESTRUYENDO;
	}

	return SOCKET_NO_ERROR;
}

int socket_connect(socket_t* this, char* address, char* port){
	int aux = 0;
	bool are_we_connected = false;
	struct addrinfo hints;
	struct addrinfo *result, *ptr;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
	hints.ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
	hints.ai_flags = 0;              /* None (or AI_PASSIVE for server) */

	aux = getaddrinfo(address, "http", &hints, &result);

	if (aux != 0) {
		printf("Error in getaddrinfo: %s\n", gai_strerror(aux));
		return SOCKET_ERROR_CONECTANDO;
	}

	for (ptr = result; ptr != NULL && are_we_connected == false; ptr = ptr->ai_next) {
		aux = connect((this->socketfd), ptr->ai_addr, ptr->ai_addrlen);
		if (aux == -1) {
			printf("Error: %s\n", strerror(errno));
		}
		are_we_connected = (aux != -1); // flag para parar cuando conectamos
	}

	freeaddrinfo(result);

	if (!are_we_connected) {
		return SOCKET_ERROR_CONECTANDO;
	}

	return SOCKET_NO_ERROR;
}

int socket_bind(socket_t* this, char* port){

}

int socket_listen(socket_t* this, int cantidadClientes){

}

int socket_send(socket_t* this, char* buffer, unsigned int size){
	int aux = 0;
	int bytes_sent = 0;
	int msg_len = strlen(buffer);
	bool is_there_a_socket_error = false;
	bool is_the_remote_socket_closed = false;

	while (bytes_sent < msg_len && is_there_a_socket_error == false && is_the_remote_socket_closed == false) {
		aux = send(this->socketfd, &buffer[bytes_sent], msg_len - bytes_sent, MSG_NOSIGNAL);

		if (aux < 0) {  // ups,  hubo un error
			printf("Error: %s\n", strerror(errno));
			is_there_a_socket_error = true;
		}
		else if (aux == 0) { // nos cerraron el socket :(
			is_the_remote_socket_closed = true;
		}
		else {
			bytes_sent += aux;
		}
	}

	if (is_there_a_socket_error || is_the_remote_socket_closed  || (aux <= 0)) {
		return SOCKET_ERROR_SEND;
	}
	return SOCKET_NO_ERROR;
}

int socket_receive(socket_t* this, char* buffer, unsigned int size){
	int aux = 0;
	int bytes_received = 0;
	int msg_len = strlen(buffer);
	bool is_there_a_socket_error = false;
	bool is_the_remote_socket_closed = false;

	while (bytes_received < msg_len && is_there_a_socket_error == false && is_the_remote_socket_closed == false) {
		aux = recv(this->socketfd, &buffer[bytes_received], msg_len - bytes_received, MSG_NOSIGNAL);

		if (aux < 0) {  // ups,  hubo un error
			printf("Error: %s\n", strerror(errno));
			is_there_a_socket_error = true;
		}
		else if (aux == 0) { // nos cerraron el socket :(
			is_the_remote_socket_closed = true;
		}
		else {
			bytes_received += aux;
		}
	}

	if (is_there_a_socket_error || is_the_remote_socket_closed  || (aux <= 0)) {
		return SOCKET_ERROR_RECEIVE;
	}
	return SOCKET_NO_ERROR;
}

int socket_shutdown(socket_t* this){
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
