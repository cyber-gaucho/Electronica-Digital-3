/* 
   Proyecto: Medición de periodo de péndulo con LPC1769
   Descripción:
     - Se utiliza un sensor en el punto de equilibrio del péndulo conectado a un 
       pin GPIO (puertos 0 ó 2, con interrupciones GPIOINT -> EINT3).
     - Cada cruce activa la interrupción y se registra el instante con SysTick 
       (configurado a 1 ms como cronómetro).
     - El periodo se calcula como la diferencia de tiempo entre dos flancos 
       consecutivos (ej. flancos de subida).
     - Los valores del periodo (en ms, tipo uint32_t) se almacenan en memoria 
       SRAM a partir de la dirección 0x2007C000, con capacidad total de 16 KB 
       (~4096 mediciones).
*/


#include "config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef unsigned int       uint32_t; // Aseguramos definición de uint32_t

void configGPIO();      // 
void configGPIOINT();   // Soporta cambios de puerto/pin
void configSystick();   // 1 ms tick

/* === VARIABLES GLOBALES (volatile cuando se usan en IRQ) === */
volatile uint32_t msTicks = 0;          /* contador global de milisegundos (SysTick) */
volatile uint32_t last_capture = 0;     /* último timestamp en ms */
volatile uint32_t storage_index = 0;    /* índice de la próxima escritura (0..STORAGE_MAX_WORDS-1) */
volatile bool first_capture_done = false;

/* Puntero al banco de memoria para almacenar periodos */
static uint32_t * const storage = (uint32_t*) MEMORY_BANK_BASE;

/* === SysTick Handler === */
void SysTick_Handler(void) {
    msTicks++;
}

/* === Función utilitaria para obtener timestamp en ms === */
static uint32_t get_ms_timestamp(void) {
    /* msTicks es 64-bit volatile; para evitar rollover parcial, hacemos doble lectura:
       en práctica con 1 ms tick y lectura simple suele ser suficiente, pero hacemos esto
       por robustez en caso de interrupciones concurrentes. */
    uint32_t a, b;
    do {
        a = msTicks;
        b = msTicks;
    } while (a != b);
    return (uint32_t)a;
}

/* === Limpiar y gestionar flags del GPIOINT ===*/
static inline void clear_gpioint_flag(uint32_t port, uint32_t pin_mask) {
    if (port == 0) {
        LPC_GPIOINT->IO0IntClr = pin_mask;
    } else { /* port == 2 */
        LPC_GPIOINT->IO2IntClr = pin_mask;
    }
}

/* === IRQ handler para GPIO interrupts (compartido bajo EINT3) ===*/
void EINT3_IRQHandler(void) {
    uint32_t pin_mask = (1 << PEND_PIN);

    /* Determinar si la interrupción fue del pin esperado:
       comprobamos el status específico del puerto.
    */
    uint32_t statR = 0, statF = 0;
    if (PEND_PORT == 0) {
        statR = LPC_GPIOINT->IO0IntStatR;  /* rising edge status */
        statF = LPC_GPIOINT->IO0IntStatF;  /* falling edge status */
    } else {
        statR = LPC_GPIOINT->IO2IntStatR;
        statF = LPC_GPIOINT->IO2IntStatF;
    }

    /* Vamos a medir entre flancos de SUBIDA consecutivos -> usamos STAT RISING */
    if (statR & pin_mask) {
        uint32_t now = get_ms_timestamp();

        if (first_capture_done) {
            uint32_t delta;
            /* cuidado con rollover de 32-bit ms (sucederá en ~49 días), pero msTicks es 64-bit
               y get_ms_timestamp devuelve 32-bit (lo cual está bien si no esperás >49 días).
               Para ser seguro, calculamos usando 64-bit desde msTicks; ya simplificamos.
            */
            delta = (now >= last_capture) ? (now - last_capture) : (uint32_t)( (uint64_t)now + (uint64_t)0x100000000ULL - last_capture );
            /* Almacenamiento: si hay espacio, guardamos, si no, simplemente dejamos de guardar */
            if (storage_index < MAX_MEASUREMENTS) {
                storage[storage_index++] = delta;
            }
            /* Si querés: podés agregar bandera para indicar overflow del banco de memoria */
        } else {
            first_capture_done = true;
        }

        last_capture = now;
        /* Limpiar flag */
        clear_gpioint_flag(PEND_PORT, pin_mask);
    }
}

/* === Inicialización del pin GPIO y GPIOINT === */
void configGPIO(void) {
    LPC_PINCON->PINSEL4 &= ~(0x3 << 2*PEND_PIN); /* P2.10 -> clear bits */
    /* Pull-down habilitado (opcional, depende del sensor) */
    LPC_PINCON->PINMODE4 |= (0x3 << 2*PEND_PIN);
    LPC_GPIO2->FIODIR &= ~(1 << PEND_PIN); /* P2.10 como entrada */
}

void configGPIOINT(void) {
    if (PEND_PORT == 0) {
        LPC_GPIOINT->IO0IntEnR |= (1u << PEND_PIN);   /* enable rising edge */
        LPC_GPIOINT->IO0IntEnF &= ~(1u << PEND_PIN); /* disable falling if solo rising */
        LPC_GPIOINT->IO0IntClr = (1u << PEND_PIN);   /* Limpiar cualquier flag preexistente */
    } else {
        LPC_GPIOINT->IO2IntEnR |= (1u << PEND_PIN);
        LPC_GPIOINT->IO2IntEnF &= ~(1u << PEND_PIN);
        LPC_GPIOINT->IO2IntClr = (1u << PEND_PIN);
    }
}

/* === Inicialización SysTick (1 ms tick) === */
static void configSystick(void) {
    if (SysTick_Config(SystemCoreClock / 1000U)) {
        SysTick->LOAD = (SystemCoreClock/1000) - 1;  // 1 us
        SysTick->VAL  = 0;
        SysTick->CTRL = (1<<0) | (1<<1) | (1<<2);       // ENABLE, TICKINT, CLKSOURCE=CPU
    }
}

/* === Función principal (ejemplo) === */
int main(void) {
    /* Opcional: configurar clocks si es necesario (suponemos SystemCoreClock inicializado). */

    /* Inicializaciones */
    configGPIO();
    configGPIOINT();
    configSystick();

    /* Inicializar índices de almacenamiento */
    storage_index = 0;
    first_capture_done = false;
    last_capture = 0;
    
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_EnableIRQ(SysTick_IRQn);
    /* Bucle principal: puede entrar en sleep para ahorrar energía; la IRQ manejará las mediciones.
       Aquí sólo hacemos un bucle infinito que podría, por ejemplo, indicar con un LED el fin
       de la adquisición o exponer una interfaz para leer la memoria.
    */
    while (1) {
        /* Podés poner WFI para ahorrar energía:
           __WFI();  // wait for interrupt
           Pero ojo con debugging y con periféricos que necesiten clocks.
        */
    }

    /* no llega acá */
    return 0;
}

