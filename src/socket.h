#ifndef SOCKET_H
#define SOCKET_H
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

typedef struct socket socket_t;

/* ******************************************************************
 *                    PRIMITIVAS DEL SOCKET
 * *****************************************************************/


/* Primitivas basicas */

// Crea un socket.
// Pre: la memoria para el socket ya fue instanciada
// Post: inicializa un socket.
int socket_init(socket_t* this, char* port);

// Destruye un socket.
// Pre: el socket esta inicializado
// Post: socket destruido
int socket_destroy(socket_t* this);

// Vincula un socket a un puerto.
// Pre: la memoria para el socket ya fue instanciada y el puerto esta libre
// Post: socket vinculado al puerto.
int socket_bind_and_listen(socket_t* this, char* port);

// Envia datos por un socket
// Pre: el socket esta inicializado
// Post: datos enviados.
int socket_send(socket_t* this, char* buffer, unsigned int size);

// Recive datos de un socket.
// Pre: el socket y el buffer estan inicializados
// Post: datos recividos.
int socket_receive(socket_t* this, char* buffer, unsigned int size);

// Cierra un socket.
// Pre: el socket esta inicializado
// Post: socket cerrado.
int socket_shutdown(socket_t* this);

// Acepta un socket cliente.
// Pre: los sockets estan instanciados e inicializados
// Post: socket cliente conectado.
int socket_accept(socket_t* this, socket_t* cliente);

#endif // SOCKET_H
