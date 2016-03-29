#ifndef SOCKET_C
#define SOCKET_C
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

int socket_init_client(socket_t* this, char* protocol, char* hostname){
	int aux;
	struct addrinfo *result, *ptr;
	bool are_we_connected = false;

	//seteo filtros de configuracion
	memset(&(this->hints), 0, sizeof(struct addrinfo));
	(this->hints).ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
	(this->hints).ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
	(this->hints).ai_flags = 0;              /* None (or AI_PASSIVE for server) */

	//busco resolver el destino
	aux = getaddrinfo(hostname, protocol, &(this->hints), &result);
	if (aux != 0) {
		printf("Error in getaddrinfo: %s\n", gai_strerror(aux));
		return SOCKET_ERROR_CREANDO;
	}

	//trato de crear el socket y conectar al servidor
	for (ptr = result; ptr != NULL && are_we_connected == false;
			ptr = ptr->ai_next) {
		aux = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (aux == -1) {
			printf("Error: %s\n", strerror(errno));
		} else {
			this->socketfd = aux;
			aux = connect(this->socketfd, ptr->ai_addr, ptr->ai_addrlen);
			if (aux == -1) {
				printf("Error: %s\n", strerror(errno));
				close(this->socketfd);
			}
			are_we_connected = (aux != -1); // dejamos de iterar cdo nos conectamos
		}
	}
	freeaddrinfo(result);

	if (are_we_connected == false) {
		return SOCKET_ERROR_CREANDO; // nos quedamos sin direcciones
	}

	return SOCKET_NO_ERROR;
}

int socket_init_server(socket_t* this, char* protocol){
	int aux;
	struct addrinfo *ptr;

	//seteo filtros de configuracion
	memset(&(this->hints), 0, sizeof(struct addrinfo));
	(this->hints).ai_family = AF_INET;       /* IPv4 (or AF_INET6 for IPv6)     */
	(this->hints).ai_socktype = SOCK_STREAM; /* TCP  (or SOCK_DGRAM for UDP)    */
	(this->hints).ai_flags = AI_PASSIVE;     /* AI_PASSIVE for server           */

	//proceso mi propia address como server
	aux = getaddrinfo(NULL, protocol, &(this->hints), &ptr);
	if (aux != 0) {
		printf("Error in getaddrinfo: %s\n", gai_strerror(aux));
		return SOCKET_ERROR_CREANDO;
	}

	//genero el socket en cuestion
	aux = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (aux==-1) {
		printf("Error: %s\n", strerror(errno));
		freeaddrinfo(ptr);
		return SOCKET_ERROR_CREANDO;
	}
	this->socketfd = aux;

	//asocio mi socket al puerto
	aux = bind(this->socketfd, ptr->ai_addr, ptr->ai_addrlen);
	if (aux == -1) {
		printf("Error: %s\n", strerror(errno));
		close(this->socketfd);
		freeaddrinfo(ptr);
		return 1;
	}

	freeaddrinfo(ptr);
	return SOCKET_NO_ERROR;
}

int socket_destroy(socket_t* this){
	int aux = close(this->socketfd);
	if (aux != 0) {
		return SOCKET_ERROR_DESTRUYENDO;
	}

	return SOCKET_NO_ERROR;
}

int socket_accept(socket_t* this, socket_t* cliente){
	int aux;
	aux = accept(this->socketfd, NULL, NULL);
	if (aux == -1) {
		printf("Error: %s\n", strerror(errno));
		return SOCKET_ERROR_ACCEPT;
	}

	cliente->socketfd = aux;
	return SOCKET_NO_ERROR;
}

int socket_listen(socket_t* this, int cantidadClientes){
	int aux;

	//pongo a escuchar clientes
	aux = listen((this->socketfd), cantidadClientes);
	if (aux == -1) {
		printf("Error: %s\n", strerror(errno));
		return SOCKET_ERROR_LISTEN;
	}

	return SOCKET_NO_ERROR;
}

int socket_send(socket_t* this, char* buffer, unsigned int size){
	int aux = 0;
	int bytes_sent = 0;
	bool is_there_a_socket_error = false;
	bool is_the_remote_socket_closed = false;

	while (bytes_sent < size &&
			is_there_a_socket_error == false &&
			is_the_remote_socket_closed == false) {
		aux = send(this->socketfd, &buffer[bytes_sent],
				size - bytes_sent, MSG_NOSIGNAL);

		if (aux < 0) {  // error al mandar
			printf("Error: %s\n", strerror(errno));
			is_there_a_socket_error = true;
		} else if (aux == 0) { // socket cerrado
			is_the_remote_socket_closed = true;
		} else {
			bytes_sent += aux;
		}
	}

	if (is_there_a_socket_error || is_the_remote_socket_closed  ||
			(aux <= 0)) {
		return SOCKET_ERROR_SEND;
	}
	return SOCKET_NO_ERROR;
}

int socket_receive(socket_t* this, char* buffer, unsigned int size){
	int aux = 0;
	int bytes_received = 0;
	bool is_there_a_socket_error = false;
	bool is_the_remote_socket_closed = false;

	while (bytes_received < size &&
			is_there_a_socket_error == false &&
			is_the_remote_socket_closed == false) {
		aux = recv(this->socketfd, &buffer[bytes_received],
				size - bytes_received, MSG_NOSIGNAL);

		if (aux < 0) {  // error al recibir
			printf("Error: %s\n", strerror(errno));
			is_there_a_socket_error = true;
		} else if (aux == 0) { // socket cerrado
			is_the_remote_socket_closed = true;
		} else {
			bytes_received += aux;
		}
	}

	if (is_there_a_socket_error || is_the_remote_socket_closed  || (aux <= 0)) {
		return SOCKET_ERROR_RECEIVE;
	}
	return SOCKET_NO_ERROR;
}

int socket_shutdown(socket_t* this) {
	shutdown(this->socketfd, SHUT_RDWR);
	return SOCKET_NO_ERROR;
}
#endif // SOCKET_C
