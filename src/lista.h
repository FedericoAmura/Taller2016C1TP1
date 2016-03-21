#ifndef LISTA_H
#define LISTA_H
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* La lista está planteada como una lista de punteros genéricos. */

typedef struct lista lista_t;
typedef struct lista_iter lista_iter_t;

/* ******************************************************************
 *                    PRIMITIVAS DE LA LISTA
 * *****************************************************************/


/* Primitivas basicas */

// Crea una lista.
// Post: devuelve una nueva lista vacía.
lista_t *lista_crear();

// Devuelve verdadero o falso, según si la lista tiene o no elementos.
// Pre: la lista fue creada.
bool lista_esta_vacia(const lista_t *lista);

// Inserta un elemento al principio de la lista. Devuelve verdadero si pudo insertarlo.
// Pre: la lista fue creada.
// Post: se agrega el elemento al principio de la lista.
bool lista_insertar_primero(lista_t *lista, void *dato);

// Inserta un elemento al final de la lista. Devuelve verdadero si pudo insertarlo.
// Pre: la lista fue creada.
// Post: se agrega el elemento al final de la lista.
bool lista_insertar_ultimo(lista_t *lista, void *dato);

// Saca el primer elemento de la lista. Si la lista tiene elementos, se quita el
// primero de la lista, y se devuelve su valor, si está vacía, devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el valor del primer elemento anterior, la lista
// contiene un elemento menos, si la lista no estaba vacía.
void *lista_borrar_primero(lista_t *lista);

// Obtiene el valor del primer elemento de la lista. Si la lista tiene
// elementos, se devuelve el valor del primero, si está vacía devuelve NULL.
// Pre: la lista fue creada.
// Post: se devolvió el primer elemento de la lista, cuando no está vacía.
void *lista_ver_primero(const lista_t *lista);

// Devuelve la cantidad de elementos que hay en la lista.
// Pre: La lista fue creada.
// Post: Se devuelve la cantidad de elementos que hay listados.
size_t lista_largo(const lista_t *lista);

// Destruye la lista. Si se recibe la función destruir_dato por parámetro,
// para cada uno de los elementos de la lista llama a destruir_dato.
// Pre: la lista fue creada. destruir_dato es una función capaz de destruir
// los datos de la lista, o NULL en caso de que no se la utilice.
// Post: se eliminaron todos los elementos de la lista.
void lista_destruir(lista_t *lista, void destruir_dato(void *));

/* Primitivas de iterador externo */

// Crea un iterador externo asociado a una lista.
// Pre: La lista fue creada.
// Post: Se devuelve un iterador apuntando al primer elemento de la lista.
// Si la lista estuviese vacia, se devuelve NULL.
lista_iter_t *lista_iter_crear(const lista_t *lista);

// Mueve el iterador al siguiente elemento de la lista.
// Pre: El iterador y la lista correspondiente fueron creados.
// Post: Se devuelve true y el iterador apunta al siguiente elemento
// o NULL en caso de estar en el final de la lista.
bool lista_iter_avanzar(lista_iter_t *iter);

// Devuelve el valor al que esta apuntando el iterador.
// Pre: El iterador y la lista correspondiente fueron creados.
// Post: Se devuelve el dato al que se encontraba apuntando el iterador.
void *lista_iter_ver_actual(const lista_iter_t *iter);

// Verifica si el iterador esta al final de la lista.
// Pre: El iterador y la lista correspondiente fueron creados.
// Post: Devuelve true si el iterador esta al final, false en caso contrario.
bool lista_iter_al_final(const lista_iter_t *iter);

// Destruye el iterador.
// Pre: El iterador fue creado.
// Post: Se libera la memoria del iterador.
void lista_iter_destruir(lista_iter_t *iter);

/* Primitivas de lista junto con iterador externo */

// Se inserta un elemento en la posicion que apunta el iterador.
// Pre: El iterador y la lista correspondiente fueron creados.
// Post: Se agrego el elemento en la posicion apuntada y quedo el iterador apuntandolo.
bool lista_insertar(lista_t *lista, lista_iter_t *iter, void *dato);

// Se devuelve y deslista el elemento al que apunta el iterador.
// Pre: El iterador y la lista correspondiente fueron creados.
// Post: Se devuelve el dato al que apuntaba el iterador luego de sacarlo de la lista.
void *lista_borrar(lista_t *lista, lista_iter_t *iter);

/* Primitivas de iterador interno */

// Ejecuta la funcion visitar a todos los elementos de la lista, el parametro extra es para esta funcion.
// Pre: La lista fue creada. La funcion visitar es acorde a los elementos de la lista.
// Post: Se procesaron con visitar todos los elementos de la lista.
void lista_iterar(lista_t *lista, bool (*visitar)(void *dato, void *extra), void *extra);

#endif // LISTA_H
