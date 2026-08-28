#include "id_reader.h"
#include <stdio.h>

static uint64_t counter = 0;
static uint16_t prefix_value = 0;  // por ejemplo 123

void id_init(uint16_t prefix, uint64_t start) {
    prefix_value = prefix % 1000;   // asegurar 3 dígitos
    counter = start;
}

uint64_t id_generate(void) {
    uint64_t id = (uint64_t)prefix_value * 1000000000000ULL + counter;
    counter++;
    return id;
}
