#include "E:\Electronica-Digital-3\common\cmsis\CMSISv2p00_LPC17xx-TRUJILLO\inc\LPC17xx.h"
#include <stdio.h>

#define BIT_MASK(x)     (1 << x)
#define BITS_MASK(x,s)  (((1 << x) - 1) << s)

#define MEMORY_BANK_BASE   (0x2007C000UL)   // UL -> Unsigned Long
#define MEMORY_BANK_SIZE   (16 * 1024U)                 // U -> Unsigned
#define MAX_MEASUREMENTS   (MEMORY_BANK_SIZE / sizeof(uint32_t))

#define PEND_PORT   2       /* Puerto GPIO: 0 o 2 (sólo 0 y 2 permiten GPIO interrupts en LPC17xx) */
#define PEND_PIN    10      /* Pin dentro del puerto (ejemplo: P2.10) */