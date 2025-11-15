#include "storage.h"
#include <stdlib.h>

/**
 * @brief Initializes the storage module
 */
void storage_init(void) {
    // Storage initialization (if needed in the future)
    // For now, this is a placeholder
}

// ---- Crear un nuevo nodo ----
Nodo* crearNodo(Registro r) {
    Nodo* nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (!nuevo) return NULL;
    nuevo->data = r;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

// ---- Insertar nuevo registro en el árbol ----
// Si el ID ya existe, actualiza los datos.
Nodo* insertarNodo(Nodo* raiz, Registro r) {
    if (raiz == NULL) return crearNodo(r);

    if (r.id < raiz->data.id)
        raiz->izq = insertarNodo(raiz->izq, r);
    else if (r.id > raiz->data.id)
        raiz->der = insertarNodo(raiz->der, r);
    else
        raiz->data = r;  // Si ya existe, se actualizan los datos

    return raiz;
}

// ---- Buscar registro por ID ----
Nodo* buscarNodo(Nodo* raiz, uint32_t id) {
    if (raiz == NULL) return NULL;
    if (id == raiz->data.id) return raiz;
    if (id < raiz->data.id) return buscarNodo(raiz->izq, id);
    return buscarNodo(raiz->der, id);
}

// ---- Recorrer árbol (por ejemplo, para exportar por USB) ----
void recorrerInOrden(Nodo* raiz) {
    if (raiz == NULL) return;
    recorrerInOrden(raiz->izq);
    // printf("ID: %lu | Tipo: %d | Estado: %d | Cat: %d | Peso: %d kg\n",
    //        raiz->data.id, raiz->data.tipo, raiz->data.estado,
    //        raiz->data.categoria, raiz->data.pesoKg);
    recorrerInOrden(raiz->der);
}