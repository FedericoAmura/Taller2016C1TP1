#ifndef LISTA_C
#define LISTA_C
#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* Se trata de una cola que contiene datos de tipo void*
 * (punteros genéricos). */

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
    lista_t* lista = malloc(sizeof(lista_t));
    if (lista==NULL) return NULL;
    lista->primero=NULL;
    lista->ultimo=NULL;
    lista->cantidad=0;
    return lista;
}

bool lista_esta_vacia(const lista_t *lista){
    return (lista->primero==NULL);
}

bool lista_insertar_primero(lista_t *lista, void *dato){
    nodo_lista_t* nuevonodo = malloc(sizeof(nodo_lista_t));
    if (nuevonodo==NULL) return false;
    nuevonodo->datos = dato;
    nuevonodo->siguiente = lista->primero;
    lista->primero = nuevonodo;
    lista->cantidad++;
    if (lista->cantidad==1) lista->ultimo=nuevonodo;
    return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato){
    nodo_lista_t* nuevonodo = malloc(sizeof(nodo_lista_t));
    if (nuevonodo==NULL) return false;
    nuevonodo->datos = dato;
    nuevonodo->siguiente = NULL;
    if (lista_esta_vacia(lista)){
    lista->primero = nuevonodo;
    }else{
    lista->ultimo->siguiente = nuevonodo;
    }
    lista->ultimo = nuevonodo;
    if (lista->cantidad==0) lista->primero = nuevonodo;
    lista->cantidad++;
    return true;
}

void *lista_borrar_primero(lista_t *lista){
    if (lista_esta_vacia(lista)) return NULL;
    void* retorno = lista_ver_primero(lista);
    nodo_lista_t* provisorio = lista->primero;
    lista->primero = lista->primero->siguiente;
    lista->cantidad--;
    if (lista->cantidad==0) lista->ultimo=NULL;
    free(provisorio);
    return retorno;
}

void *lista_ver_primero(const lista_t *lista){
    if (lista->primero==NULL) return NULL;
    return lista->primero->datos;
}

size_t lista_largo(const lista_t *lista){
    return lista->cantidad;
}

void lista_destruir(lista_t *lista, void destruir_dato(void *)){
    while(!lista_esta_vacia(lista)){
        if (destruir_dato!=NULL) destruir_dato(lista->primero->datos);
        lista_borrar_primero(lista);
    }
    free(lista);
}

lista_iter_t *lista_iter_crear(const lista_t *lista){
    lista_iter_t* iter = malloc(sizeof(lista_iter_t));
    if (iter==NULL) return NULL;
    iter->anterior = NULL;
    iter->actual = lista->primero;
    return iter;
}

bool lista_iter_avanzar(lista_iter_t *iter){
    if (iter->actual==NULL) return false;
    iter->anterior=iter->actual;
    iter->actual=iter->actual->siguiente;
    return true;
}

void *lista_iter_ver_actual(const lista_iter_t *iter){
    if (iter->actual==NULL) return NULL;
    return iter->actual->datos;
}

bool lista_iter_al_final(const lista_iter_t *iter){
    if (iter->actual==NULL) return true;
    return false;
}

void lista_iter_destruir(lista_iter_t *iter){
    free(iter);
}

bool lista_insertar(lista_t *lista, lista_iter_t *iter, void *dato){
    if (dato==NULL) return false;
    nodo_lista_t* nuevonodo = malloc(sizeof(nodo_lista_t));
    if (nuevonodo==NULL) return false;
    nuevonodo->datos=dato;
    if (iter->actual==NULL){
        lista->ultimo=nuevonodo;
        nuevonodo->siguiente=NULL;
    }else{
        nuevonodo->siguiente=iter->actual;
    }
    iter->actual=nuevonodo;
    lista->cantidad++;
    if (lista->cantidad==1) lista->primero=nuevonodo;
    if (iter->anterior==NULL){
        lista->primero = nuevonodo;
    }else{
        iter->anterior->siguiente=nuevonodo;
    }
    return true;
}

void *lista_borrar(lista_t *lista, lista_iter_t *iter){
    if (iter->actual==NULL) return NULL;
    void* dato = iter->actual->datos;
    if (iter->anterior==NULL){
        lista->primero = iter->actual->siguiente;
        free(iter->actual);
        iter->actual = lista->primero;
    }else{
        iter->anterior->siguiente = iter->actual->siguiente;
        free(iter->actual);
        if (iter->anterior->siguiente!=NULL){
            iter->actual = iter->anterior->siguiente;
        }else{iter->actual=NULL;};
    }
    lista->cantidad--;
    if (lista->cantidad==1) lista->ultimo=lista->primero;
    if (lista->cantidad==0) lista->ultimo=NULL;
    return dato;
}

void lista_iterar(lista_t *lista, bool (*visitar)(void *dato, void *extra), void *extra){
    bool seguir = true;
    void* datoactual;
    lista_iter_t* iter=lista_iter_crear(lista);
    if (iter==NULL) return;
    while(!(lista_iter_al_final(iter)) && seguir){
        datoactual = lista_iter_ver_actual(iter);
        seguir = visitar(datoactual,extra);
        lista_iter_avanzar(iter);
    }
    lista_iter_destruir(iter);
}

#endif // LISTA_C
