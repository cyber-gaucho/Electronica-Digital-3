#ifndef STORAGE_H
#define STORAGE_H

// --- Estructura de datos ---
typedef struct {
    uint32_t id;       // ID leído del RFID
    uint8_t tipo;      // índice de tipo seleccionado
    uint8_t estado;    // índice de estado
    uint8_t categoria; // índice de categoría
    int pesoKg;        // peso cargado
} Registro;

// ---- Nodo del árbol binario ----
typedef struct Nodo {
    Registro data;
    struct Nodo* izq;
    struct Nodo* der;
} Nodo;

void storage_init(void);
Nodo* crearNodo(Registro r);
Nodo* insertarNodo(Nodo* raiz, Registro r);
Nodo* buscarNodo(Nodo* raiz, uint32_t id);
void recorrerInOrden(Nodo* raiz);

#endif /* STORAGE_H */