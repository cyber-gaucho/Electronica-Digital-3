#ifndef ID_READER_H
#define ID_READER_H

#include <stdint.h>

void id_init(uint16_t prefix, uint64_t start);
uint64_t id_generate(void);

#endif
