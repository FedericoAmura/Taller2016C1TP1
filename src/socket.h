#ifndef SOCKET_H
#define SOCKET_H


/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* Se trata de un socket simplificado */

struct socket {
	int socketfd;	//file descriptor del socket
	struct addrinfo hints;	//estructura filtro de conexiones
};

typedef struct socket socket_t;

/* ******************************************************************
 *                    PRIMITIVAS DEL SOCKET
 * *****************************************************************/


/* Primitivas basicas */

// Crea un socket tipo cliente y lo conecta hacia un servidor
// Pre: la memoria para el socket ya fue instanciada
// Post: socket inicializado y conectado
int socket_init_client(socket_t* this, char* protocol, char* hostname);

// Crea un socket tipo servidor vinculado a un puerto local
// Pre: la memoria para el socket ya fue instanciada
// Post: inicializa un socket en el puerto especificado
int socket_init_server(socket_t* this, char* protocol);

// Destruye un socket.
// Pre: el socket esta inicializado
// Post: socket destruido
int socket_destroy(socket_t* this);

// Acepta un socket cliente.
// Pre: los sockets estan instanciados e inicializados
// Post: socket cliente conectado.
int socket_accept(socket_t* this, socket_t* cliente);

// Pone un socket a escuchar conexiones de su puerto
// Pre: socket listo, vinculado al puerto
// Post: socket escuchando tantas conexiones como se especifico
int socket_listen(socket_t* this, int cantidadClientes);

// Envia datos por un socket
// Pre: el socket esta inicializado
// Post: datos enviados.
int socket_send(socket_t* this, char* buffer, unsigned int size);

// Recive datos de un socket.
// Pre: el socket y el buffer estan inicializados
// Post: datos recividos.
int socket_receive(socket_t* this, char* buffer, unsigned int size);

#endif // SOCKET_H
