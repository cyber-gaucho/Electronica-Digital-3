#include "storage.h"
#include "utils.h"
#include <string.h>

#define STORAGE_MAX    1200

static Registro registros[STORAGE_MAX];
static uint16_t registrosCount = 0;

void storage_init(void) {
    registrosCount = 0;
}

static void insertarRegistro(Registro r) {
    if (registrosCount >= STORAGE_MAX)
        return;

    registros[registrosCount++] = r;
}

Registro* buscarRegistro(uint64_t id) {
    for (uint16_t i = 0; i < registrosCount; i++) {
        if (registros[i].id == id)
            return (Registro*)&registros[i];
    }
    return NULL;
}

Registro* buscarRegistroIndex(uint16_t ind) {
	return (Registro*)&registros[ind];
}

/* Guarda un nuevo dato */
void storage_guardarDato(uint64_t id, uint8_t raza, uint8_t categoria, uint8_t origen, uint16_t pesoKg) {
    Registro r;
    r.id = id;
    r.raza = raza;
    r.categoria = categoria;
    r.origen = origen;
    r.pesoKg = pesoKg;

    insertarRegistro(r);
}
