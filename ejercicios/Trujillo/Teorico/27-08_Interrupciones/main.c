/*
 * Ejercicio de interrupciones:
 * - Un LED parpadea cada 500 ms (SysTick).
 * - Al presionar un botón (EINT2), el parpadeo se detiene y el LED se apaga.
 * - Al presionar otro botón (GPIO P2.0), el parpadeo se reanuda.
 *
 * Configuraciones:
 * - LED en P0.22 (activo por bajo).
 * - Botón para EINT2 en P2.12 (flanco de bajada).
 * - Botón para GPIO P2.0 (flanco de subida).
 *
 * Fecha: 27-08-2024
 */


#include "LPC17xx.h"
#include <stdint.h>
#include "config.h"

// ---------------- VARIABLES GLOBALES ----------------
volatile uint8_t enable_blink = 1;   // flag para habilitar o no el parpadeo
volatile uint8_t led_state = 0;      // estado actual del LED

// Prototipos de funciones
void configGPIO(void);
void configST(void);
void configPortInt(void);
void configEINT(void);

void SysTick_Handler(void);
void EINT2_IRQHandler(void);
void EINT3_IRQHandler(void);

// ---------------- CONFIGURACIONES ----------------

// Configuración del GPIO (LED y pin de interrupción)
void configGPIO(void) {
    // LED en P0.22 como salida
    LPC_PINCON->PINSEL1 &= ~(LED_MASK_PINSEL);    // P0.22 GPIO
    LPC_GPIO0->FIODIR   |= LED_MASK;              // salida
    LPC_GPIO0->FIOSET   |= LED_MASK;              // Inicialmente apagado (activo por bajo)

    // Pin P2.0 como entrada
    LPC_PINCON->PINSEL4 &= ~(GPIO_INT_PINSEL);    // P2.0 GPIO
    LPC_GPIO2->FIODIR  &= ~GPIO_INT_MASK;         // entrada
}

// Configuración del SysTick (parpadeo cada 500 ms)
void configST(void) {
    SystemCoreClockUpdate();
    SysTick->LOAD = (SystemCoreClock / 2) - 1; // 500 ms (para Fclk = 100 MHz → 50M ticks)
    SysTick->VAL  = 0;
    SysTick->CTRL = (1 << 0) | (1 << 1) | (1 << 2); // ENABLE, TICKINT, CLKSOURCE
}

// Configuración de interrupción externa por puerto (GPIO P2.0)
void configPortInt(void) {
    // Interrupción por flanco de subida en P2.0
    LPC_GPIOINT->IO2IntEnR |= GPIO_INT_MASK;
    LPC_GPIOINT->IO2IntClr  = GPIO_INT_MASK;

    NVIC_EnableIRQ(EINT3_IRQn); // EINT3 maneja las interrupciones GPIO
}

// Configuración de EINT2
void configEINT(void) {
    LPC_PINCON->PINSEL4 |= (EINT2_PINSEL);  // P2.12 como EINT2
    LPC_SC->EXTMODE  |= (1 << 2);      // sensible a flanco
    LPC_SC->EXTPOLAR &= ~(1 << 2);     // flanco de bajada
    LPC_SC->EXTINT   |= (1 << 2);      // limpiar flag

    NVIC_EnableIRQ(EINT2_IRQn);
}

// ---------------- HANDLERS ----------------

// SysTick cada 500 ms -> toggle LED
void SysTick_Handler(void) {
    if (enable_blink) {
        LPC_GPIO0->FIOPIN ^= LED_MASK;  // Toggle LED
        led_state ^= 1;                // Toggle estado
    }
}

// EINT2 -> deshabilita el parpadeo
void EINT2_IRQHandler(void) {
    enable_blink = 0;
    LPC_GPIO0->FIOSET = LED_MASK;  // asegurar apagado
    led_state = 0;

    LPC_SC->EXTINT = (1 << 2);     // limpiar flag
}

// GPIO P2.0 -> habilita nuevamente el parpadeo
void EINT3_IRQHandler(void) {
    if (LPC_GPIOINT->IO2IntStatR & GPIO_INT_MASK) {
        enable_blink = 1;
        LPC_GPIOINT->IO2IntClr = GPIO_INT_MASK; // limpiar flag
    }
}

// ---------------- MAIN ----------------
int main(void) {
    configGPIO();
    configST();
    configEINT();
    configPortInt();

    while (1) {
        __WFI();  // espera interrupción
    }
}
