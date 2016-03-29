#ifndef LISTA_C
#define LISTA_C
#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#define LISTA_NO_ERROR 0
#define LISTA_ERROR 1

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* Se trata de una cola basica que contiene datos de tipo void* */

typedef struct nodo_lista {
    void* datos;
    struct nodo_lista* siguiente;
}nodo_lista_t;

struct lista {
    struct nodo_lista* primero;
    struct nodo_lista* ultimo;
    size_t cantidad;
};

struct lista_iter {
    struct nodo_lista* actual;
    struct nodo_lista* anterior;
};

/* ******************************************************************
 *                    PRIMITIVAS DE LA LISTA
 * *****************************************************************/

lista_t *lista_crear(){
    lista_t* lista = calloc(1,sizeof(lista_t));
    if (lista==NULL) return NULL;
    return lista;
}

bool lista_esta_vacia(const lista_t *lista){
    return (lista->cantidad==0);
}

int lista_insertar_ultimo(lista_t *lista, void *dato){
    nodo_lista_t* nuevoNodo = calloc(1,sizeof(nodo_lista_t));
    if (nuevoNodo==NULL) return LISTA_ERROR;
    nuevoNodo->datos = dato;
    if (lista_esta_vacia(lista)){
    	lista->primero = nuevoNodo;
    }else{
    	lista->ultimo->siguiente = nuevoNodo;
    }
    lista->ultimo = nuevoNodo;
    lista->cantidad++;
    return LISTA_NO_ERROR;
}

void *lista_borrar_primero(lista_t *lista){
    if (lista_esta_vacia(lista)) return NULL;
    void* retorno;
    if (lista->primero == NULL) return NULL;
    retorno = lista->primero->datos;
    nodo_lista_t* aux = lista->primero;
    lista->primero = lista->primero->siguiente;
    lista->cantidad--;
    if (lista->cantidad==0) lista->ultimo=NULL;
    free(aux);
    return retorno;
}

void lista_destruir(lista_t *lista){
    free(lista);
}

lista_iter_t *lista_iter_crear(const lista_t *lista){
    lista_iter_t* iter = calloc(1,sizeof(lista_iter_t));
    if (iter==NULL) return NULL;
    iter->actual = lista->primero;
    return iter;
}

int lista_iter_avanzar(lista_iter_t *iter){
    if (iter->actual==NULL) return LISTA_ERROR;
    iter->anterior=iter->actual;
    iter->actual=iter->actual->siguiente;
    return LISTA_NO_ERROR;
}

void *lista_iter_ver_actual(const lista_iter_t *iter){
    if (iter->actual==NULL) return NULL;
    return iter->actual->datos;
}

void lista_iter_destruir(lista_iter_t *iter){
    free(iter);
}

#endif // LISTA_C
