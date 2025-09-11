/*
 Medición de periodo y control de parpadeo de LED
   - Se utiliza la interrupción externa EINT1 para capturar flancos de subida
     de una señal cuadrada y medir su periodo.
   - El periodo medido se limita al rango [100 ms, 1000 ms].
   - Dicho periodo se utiliza para controlar la frecuencia de parpadeo de un LED,
     con cambio de estado cada periodo/2.
   - Se emplean únicamente interrupciones: SysTick (base de tiempo) y EINT1 
     (captura de flancos).
   - Se define la prioridad relativa de las interrupciones:
       * EINT1 con mayor prioridad (para no perder flancos).
       * SysTick con prioridad menor (su retraso no afecta el funcionamiento).
*/

#include "E:\Electronica-Digital-3\common\cmsis\CMSISv2p00_LPC17xx-TRUJILLO\inc\LPC17xx.h"
//typedef unsigned int       uint32_t; // Corregimos definición de uint32_t

#define LED_PIN     22      // P0.22 -> LED
#define LED_PORT    LPC_GPIO0
#define EINT1_PIN   11      // P2.11 -> EINT1

volatile uint32_t systick_ms = 0;     // contador global de tiempo (ms)
volatile uint32_t t_ultimo = 0;       // último flanco
volatile uint32_t periodo = 500;      // periodo medido (ms), arranca en 500 por defecto
volatile uint32_t led_counter = 0;    // contador para LED
volatile uint8_t led_state = 0;       // estado LED

// SysTick cada 1 ms
void SysTick_Handler(void) {
    systick_ms++;
    
    led_counter++;
    if (led_counter >= (periodo / 2)) { // con periodo/2 replica el comportamiento de la señal
        led_counter = 0;
        led_state = !led_state;
        if (led_state)
            LED_PORT->FIOSET = (1 << LED_PIN);  // LED ON
        else
            LED_PORT->FIOCLR = (1 << LED_PIN);  // LED OFF
    }
}

void EINT1_IRQHandler(void) {
    uint32_t t_actual = systick_ms;
    uint32_t delta = t_actual - t_ultimo;
    t_ultimo = t_actual;
    
    // Ajustar periodo entre 100 y 1000 ms
    if (delta < 100) delta = 100;
    if (delta > 1000) delta = 1000;
    periodo = delta;
    
    LPC_SC->EXTINT = (1 << 1);  // Limpiar flag EINT1
}

int main(void) {
    LED_PORT->FIODIR |= (1 << LED_PIN);

    // Configuración SysTick a 1ms
    SysTick_Config(SystemCoreClock / 1000); // Prioridad por defecto (más baja)

    // Configuración EINT1 en P2.11
    LPC_PINCON->PINSEL4 |= (1 << EINT1_PIN*2);  // P2.11 como EINT1
    LPC_SC->EXTMODE |= (1 << 1);       // Flanco
    LPC_SC->EXTPOLAR |= (1 << 1);      // Flanco de subida
    NVIC_EnableIRQ(EINT1_IRQn);

    while (1) {
        // todo se maneja en interrupciones
    }
}
