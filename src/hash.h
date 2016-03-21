#ifndef HASH_H
#define HASH_H
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* El hash esta planteado como un vector de listas con la ubicacion dada segun clave. */

typedef struct hash hash_t;
typedef struct hash_iter hash_iter_t;
typedef void (*hash_destruir_dato_t)(void *);

/* Se trata de un hash abierto donde las colisiones van a parar a una estructura anexa */

/* ******************************************************************
 *                    PRIMITIVAS DEL HASH
 * *****************************************************************/

/* Primitivas basicas */

// Crea un hash.
// Post: Devuelve un nuevo hash vacio.
hash_t *hash_crear(hash_destruir_dato_t destruir_dato);

// Devuelve verdadero o falso, segun si el hash contiene la clave.
// Pre: El hash fue creado.
bool hash_pertenece(const hash_t *hash, const char *clave);

// Agrega un dato y su clave asociada al hash.
// Pre: El hash fue creado.
// Post: 
bool hash_guardar(hash_t *hash, const char *clave, void *dato);

// Borra el elemento asociado a la clave.
// Pre: El hash fue creado.
// Post: Se devuelve el elemento de la clave asociada y
// se destruye su almacenamiento en el hash.
void *hash_borrar(hash_t *hash, const char *clave);

// Obtiene el elemento del hash con la clave asociada.
// Pre: El hash fue creado.
// Post: Devuelve el elemento de la clave asociada.
void *hash_obtener(const hash_t *hash, const char *clave);

// Devuelve la cantidad de elementos del hash.
// Pre: El hash fue creado.
// Post: Devuelve la cantidad de elementos en el hash
size_t hash_cantidad(const hash_t *hash);

// Destruye el hash.
// Pre: El hash fue creado.
// Post: Se destruyo el hash y todos sus datos almacenados.
void hash_destruir(hash_t *hash);

/* Primitivas de iterador externo */

// Crea un iterador asociado al hash.
// Pre: El hash fue creado.
// Post: Devuelve un iterador apuntando al primer elemento del hash.
hash_iter_t *hash_iter_crear(const hash_t *hash);

// Mueve el iterador hacia el siguiente elemento.
// Pre: El hash fue creado.
// Post: El iterador ahora apunta al siguiente elemento.
bool hash_iter_avanzar(hash_iter_t *iter);

// Devuelve la clave del elemento al que esta apuntando el iterador.
// Pre: El hash fue creado.
// Post: Se devuelve la clave apuntada.
const char *hash_iter_ver_actual(const hash_iter_t *iter);

// Verifica si el iterador se encuentra al final del hash..
// Pre: El hash fue creado.
// Post: Devuelve verdadero si el iterador esta al final del hash.
bool hash_iter_al_final(const hash_iter_t *iter);

// Destruye el iterador.
// Pre: El hash fue creado.
// Post: El iterador se destruyo.
void hash_iter_destruir(hash_iter_t* iter);

#endif // HASH_H
