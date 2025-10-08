#include "LPC17xx.h"

#define SALIDA_PIN    (1<<4)   // P2.4
#define SALIDA_PORT    LPC_GPIO2

volatile int secuencia_activa = 0;
volatile int ticks = 0;

void SysTick_Handler(void) {
    if(secuencia_activa)
        ticks++;
}

void EINT2_IRQHandler(void) {
    // Limpiar bandera
    LPC_SC->EXTINT |= (1<<2);

    if(secuencia_activa) {
        // interrumpir secuencia
        SALIDA_PORT->FIOSET = SALIDA_PIN;
        secuencia_activa = 0;
        ticks = 0;  // 
    } else {
        // iniciar nueva secuencia
        secuencia_activa = 1;
    }
}

int main(void) {
    // Config pin como salida
    SALIDA_PORT->FIODIR |= SALIDA_PIN;

    // Config EINT2 en P2.12
    LPC_PINCON->PINSEL4 |= (1<<24); // P2.12 como EINT2
    LPC_SC->EXTMODE |= (1<<2);      // edge sensitive
    LPC_SC->EXTPOLAR &= ~(1<<2);    // flanco descendente

    NVIC_EnableIRQ(EINT2_IRQn);

    // Configuración SysTick a 1ms
    SysTick_Config(SystemCoreClock / 1000); // Prioridad por defecto (más baja)


    while(1) {

        if(secuencia_activa) {  // se puede mover al SysTick Handler
            if(ticks == 4 || ticks == 6 || ticks == 8 || ticks == 10) {
                SALIDA_PORT->FIOSET |= SALIDA_PIN;
            }
            else {
                SALIDA_PORT->FIOCLR |= SALIDA_PIN;
                if(ticks == 14) {
                    // fin de secuencia
                    secuencia_activa = 0;
                    ticks = 0;
                }   //LA SALIDA QUEDA EN 0
            }
        }

    }
}
