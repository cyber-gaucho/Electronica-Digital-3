#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>
// --- Estructura de datos ---
typedef struct {
    uint64_t id;       	// ID leído del RFID
    uint8_t raza;      	// índice de tipo seleccionado
    uint8_t categoria;  // índice de estado
    uint8_t origen; 	// índice de categoría
    uint16_t pesoKg;    // peso cargado
} Registro;

void storage_init(void);
void storage_guardarDato(uint64_t id, uint8_t raza,
				uint8_t categoria, uint8_t origen, uint16_t pesoKg);

Registro* buscarRegistro(uint64_t id);
Registro* buscarRegistroIndex(uint16_t ind);

#endif /* STORAGE_H */