#ifndef HASH_C
#define HASH_C
#include "hash.h"
#include "lista.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#define TAMANIOINICIAL 101
#define FACTORCARGA 0.7

/* ******************************************************************
 *                DEFINICION DE LOS TIPOS DE DATOS
 * *****************************************************************/

/* Se trata de un hash abierto implementado como un vector de listas. */

struct hash{
    size_t cantidad;
    size_t tamanio;
    hash_destruir_dato_t destruir_dato;
    lista_t** tabla;
};

struct hash_iter{
    lista_iter_t* iter;
    const hash_t* hash;
    size_t posicion;
};

typedef struct nodo_hash{
    char* clave;
    void* dato;
} nodo_hash_t;


/* ******************************************************************
 *                    PRIMITIVAS DEL HASH
 * *****************************************************************/

// Funcion auxiliar que se utiliza para mapear la clave
size_t fhash(const char* clave, size_t tamanio) {
    unsigned int h = 0;
    for(unsigned int i = 0; i<strlen(clave);i++) h=31*h+clave[i];
    return h%tamanio;
}

// Funcion auxiliar que busca la clave en la lista y devuelve el nodo
nodo_hash_t* buscar_clave(lista_t* lista, const char* clave) {
    lista_iter_t* iter = lista_iter_crear(lista);
    bool esta = false;
    nodo_hash_t* nodo;
    do {
        nodo = (nodo_hash_t*)lista_iter_ver_actual(iter);
        if (nodo != NULL) {
            if (strcmp(clave,nodo->clave) == 0) {
                esta = true;
            }
        }
    } while(lista_iter_avanzar(iter) && !esta);
    if (esta == false) {
	lista_iter_destruir(iter);
        return NULL;
    }
    lista_iter_destruir(iter);
    if (nodo == NULL) return NULL;
    return nodo;
}

// Funcion auxiliar que crea un nuevo nodo para el hash
nodo_hash_t* creacion_nodo(const char* clave, void* dato) {
    nodo_hash_t* nodo_hash = malloc(sizeof(nodo_hash_t));
    if (nodo_hash == NULL) return NULL;
    //agrego el +1 por el /0
    nodo_hash->clave = malloc(sizeof(char)*(strlen(clave)+1));
    strcpy(nodo_hash->clave,clave);
    nodo_hash->dato = dato;
    return nodo_hash;
}

// Funcion auxiliar para la creacion de la tabla
lista_t** creacion_tabla(hash_t* hash, size_t tamanio) {
    // pido memoria para la tabla y me aseguro de que no sea NULL
    lista_t** tabla = calloc(tamanio,sizeof(lista_t*));
    if (tabla == NULL) {
	free(hash);
	return NULL;
    }
    // creo listas para cada posicion de la tabla
    for (int i = 0; i < tamanio; i ++) {
	tabla[i] = lista_crear();
    }
    return tabla;
}


// Funcion auxiliar para crear un hash de un tamanio variable
hash_t* creacion_hash(hash_destruir_dato_t destruir_dato, size_t tamanio) {
    hash_t* hash = malloc(sizeof(hash_t));
    if (hash == NULL) return NULL;
    lista_t** tabla = creacion_tabla(hash,tamanio);
    if (tabla == NULL) return NULL;
    hash->tabla = tabla;
    hash->cantidad = 0;
    hash->tamanio = tamanio;
    hash->destruir_dato = destruir_dato;
    return hash;
}

hash_t* hash_crear(hash_destruir_dato_t destruir_dato) {
    hash_t* hash = creacion_hash(destruir_dato,TAMANIOINICIAL);
    return hash;
}

hash_t* hash_redimensionar(hash_t* hash, size_t tamanio) {
    lista_t** nueva_tabla = creacion_tabla(hash,tamanio);
    if (nueva_tabla == NULL) return NULL;
    for (int i = 0; i < hash->tamanio; i ++) {
        lista_iter_t* iter = lista_iter_crear(hash->tabla[i]);
        const char* clave;
        size_t posicion;
        nodo_hash_t* nodo;
        while (!lista_iter_al_final(iter)) {
            nodo = lista_borrar(hash->tabla[i],iter);
	    clave = nodo->clave;
	    posicion = fhash(clave,tamanio);
	    lista_insertar_primero(nueva_tabla[posicion],nodo);
        }
        lista_destruir(hash->tabla[i],NULL);
	lista_iter_destruir(iter);
    }

    free(hash->tabla);
    hash->tamanio = tamanio;
    hash->tabla = nueva_tabla;
    return hash;
}


bool hash_guardar(hash_t* hash, const char* clave, void* dato) {
    size_t posicion = fhash(clave,hash->tamanio);
    nodo_hash_t* nodo_clave = buscar_clave(hash->tabla[posicion],clave);
    // si la clave no existe
    if (nodo_clave == NULL) {
        nodo_hash_t* nodo = creacion_nodo(clave,dato);
        lista_insertar_primero(hash->tabla[posicion],nodo);
        hash->cantidad ++;
    }
    // si la clave ya existe, reemplazo el dato
    if (nodo_clave != NULL) {
        if (hash->destruir_dato != NULL) {
            hash->destruir_dato(nodo_clave->dato);
        }
        nodo_clave->dato = dato;
    }

    //Factor de carga de 70 %
    float resultado = (hash->cantidad)/(hash->tamanio);
    if (resultado > FACTORCARGA)
        hash_redimensionar(hash,(hash->tamanio)*2);
    return true;

}

bool hash_pertenece(const hash_t* hash, const char* clave) {
    size_t posicion = fhash(clave,hash->tamanio);
    nodo_hash_t* nodo_clave = buscar_clave(hash->tabla[posicion],clave);
    // si no encuentro la clave devuelvo NULL
    if (nodo_clave == NULL) {
        return false;
    }
    // si encuentro la clave devuelvo true
    return true;
}

void* hash_borrar(hash_t* hash, const char* clave) {
    if (!hash_pertenece(hash,clave)) return NULL;
    size_t posicion = fhash(clave,hash->tamanio);
    lista_iter_t* iter = lista_iter_crear(hash->tabla[posicion]);
    nodo_hash_t* nodo = lista_iter_ver_actual(iter);
    bool esta = false;
    while (!lista_iter_al_final(iter)) {
        if (strcmp(nodo->clave,clave) == 0) {
 	    esta = true;
	    break;
        }
        lista_iter_avanzar(iter);
        nodo = lista_iter_ver_actual(iter);
    }

    if (esta == false) {
	lista_iter_destruir(iter);
	return NULL;
    }
    nodo = (nodo_hash_t*)lista_borrar(hash->tabla[posicion],iter);
    void* dato = nodo->dato;
    free(nodo->clave);
    free(nodo);
    lista_iter_destruir(iter);
    hash->cantidad --;
    return dato;
}

void* hash_obtener(const hash_t* hash, const char* clave) {
    size_t posicion = fhash(clave,hash->tamanio);
    nodo_hash_t* nodo_clave = buscar_clave(hash->tabla[posicion],clave);
    // si no encuentro la clave devuelvo NULL
    if (nodo_clave == NULL) {
        return NULL;
    }
    // si encuentro la clave devuelvo el dato
    return nodo_clave->dato;
}

size_t hash_cantidad(const hash_t* hash) {
    return hash->cantidad;
}

void hash_destruir(hash_t* hash) {
    for (int i = 0; i < hash->tamanio; i ++) {
	lista_iter_t* iter = lista_iter_crear(hash->tabla[i]);
	while (!lista_iter_al_final(iter)) {
	    nodo_hash_t* nodo = lista_borrar(hash->tabla[i],iter);
	    if (hash->destruir_dato) {
		hash->destruir_dato(nodo->dato);
	    }
	    free(nodo->clave);
	    free(nodo);
	}
	lista_iter_destruir(iter);
	lista_destruir(hash->tabla[i],NULL);
    }
    free(hash->tabla);
    free(hash);
}


/* ******************************************************************
 *                  PRIMITIVAS DEL ITERADOR
 * *****************************************************************/

// Funcion auxiliar que busca la primer lista no vacia y devuelve su posicion
size_t buscar_no_vacia(const hash_t* hash, size_t posicion) {
    while (lista_esta_vacia(hash->tabla[posicion]) && (posicion < hash->tamanio - 1)) {
	posicion++;
    }
    return posicion;
}

hash_iter_t* hash_iter_crear(const hash_t* hash) {
    hash_iter_t* iter = malloc(sizeof(hash_iter_t));
    if (iter == NULL) return NULL;
    //busco la primer lista no vacia de la tabla
    size_t posicion = buscar_no_vacia(hash,0);
    //creo un iterador de lista en dicha lista
    lista_iter_t* lista_iter = lista_iter_crear(hash->tabla[posicion]);
    iter->iter = lista_iter;
    iter->hash = hash;
    iter->posicion = posicion;
    return iter;
}

bool hash_iter_al_final(const hash_iter_t* iter) {
    return (lista_iter_al_final(iter->iter));

}

bool hash_iter_avanzar(hash_iter_t* iter) {
    //verifico no estar ya al final
    if (hash_iter_al_final(iter)) return false;
    //avanzo y verifico no estar ahora al final
    lista_iter_avanzar(iter->iter);
    if (!lista_iter_al_final(iter->iter)) return true;
    //como estoy al final de la lista, busco si tengo a donde seguir
    if (iter->posicion == (iter->hash->tamanio)-1) return false;
    //como lo tengo, busco
    size_t posicion = buscar_no_vacia(iter->hash,iter->posicion+1);
    if (!lista_esta_vacia(iter->hash->tabla[posicion])) {
        lista_iter_destruir(iter->iter);
        iter->iter = lista_iter_crear(iter->hash->tabla[posicion]);
        iter->posicion = posicion;
        return true;
    }
    return false;
}

const char* hash_iter_ver_actual(const hash_iter_t* iter) {
    if (hash_iter_al_final(iter)) return NULL;
    nodo_hash_t* nodo = lista_iter_ver_actual(iter->iter);
    if (nodo != NULL)
        return (nodo->clave);
    return NULL;
}

void hash_iter_destruir(hash_iter_t* iter) {
    lista_iter_destruir(iter->iter);
    free(iter);
}

#endif // HASH_C
