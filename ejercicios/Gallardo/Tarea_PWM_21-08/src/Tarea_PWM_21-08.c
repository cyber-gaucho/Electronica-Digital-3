/*
Este proyecto lee los pines 0, 1, 2 y 3 del puerto GPIO0
y saca una señal PWM por el pin 22 del mismo puerto, cuyo
% de duty cycle depende del valor (0-15) de las entradas y
su frecuencia es de 20 hercios.
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include <stdio.h>
#include "config.h"


void configGPIO(void);
void delay(uint32_t ms);
uint8_t getDutyCycle(void);

int main(void) {
//    SystemInit();
    configGPIO();
    uint8_t duty = 0, last_duty = 0x0F;
    uint32_t periodo_ms = 1000 / PWM_FREQ_HZ;

    while (1) {
        duty = getDutyCycle(); // Se podría hacer con interrupciones por GPIO
        if (duty != last_duty) {
            printf("Duty Cycle: %d%%\n", (100 * duty) / PWM_MAX_DUTY);
            last_duty = duty;
        }
        uint32_t tiempo_alto = (periodo_ms * duty) / PWM_MAX_DUTY;
        uint32_t tiempo_bajo = periodo_ms - tiempo_alto;

        LPC_GPIO0->FIOSET |= PWM_PIN_MASK;
        delay(tiempo_alto);
        LPC_GPIO0->FIOCLR |= PWM_PIN_MASK;
        delay(tiempo_bajo);
    }
    return 0;
}

void configGPIO(void) {
    // Configura P0.[3:0] como entradas
    LPC_PINCON->PINSEL0 &= ~(GPIO0_BIT0_DB | GPIO0_BIT1_DB | GPIO0_BIT2_DB | GPIO0_BIT3_DB);
    LPC_GPIO0->FIODIR &= ~(GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3);

    // Configura P0.22 como salida para la señal PWM
    LPC_PINCON->PINSEL1 &= ~PWM_PIN_MASK_DB;
    LPC_GPIO0->FIODIR |= PWM_PIN_MASK;
    LPC_GPIO0->FIOCLR |= PWM_PIN_MASK;
}

uint8_t getDutyCycle(void) {
/*
 * Sintaxis del operador ternario (?:)
 * Expresión boleana ? valor si cierto : valor si falso
 */
//    uint8_t pin0 = (LPC_GPIO0->FIOPIN & BIT0) ? 1 : 0;
//    uint8_t pin1 = (LPC_GPIO0->FIOPIN & BIT1) ? 1 : 0;
//    uint8_t pin2 = (LPC_GPIO0->FIOPIN & BIT2) ? 1 : 0;
//    uint8_t pin3 = (LPC_GPIO0->FIOPIN & BIT3) ? 1 : 0;
//
//    uint8_t duty_cycle = (pin3 << 3) | (pin2 << 2) | (pin1 << 1) | pin0;
//
//    if (duty_cycle > PWM_MAX_DUTY) {
//        duty_cycle = PWM_MAX_DUTY;
//    }
//
//    return duty_cycle;

	//Se puede hacer solo porque los pines 0-3 son contiguos
    uint8_t value = LPC_GPIO0->FIOPIN & 0x0F;


    if (value > PWM_MAX_DUTY) {
        value = PWM_MAX_DUTY;
    }
    return value;
}

void delay(uint32_t ms) {
    volatile uint32_t count;
    while (ms--) {
        for (count = 0; count < 10000; count++) {
            __asm__("nop");
        }
    }
}
