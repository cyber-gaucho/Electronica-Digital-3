/**
 * Este proyecto lee los pines 0, 1, 2 y 3 del puerto GPIO0
 * y saca una señal PWM por el pin 22 del mismo puerto, cuyo
 * % de duty cycle depende del valor (0-15) de las entradas y
 * su frecuencia es de 20 hercios.
 * Además, al pulsar el botón conectado a EINT3, se habilita
 * o deshabilita la salida PWM.
 */

#include "config.h"

void configGPIO(void);
void configGPIOInterrupt(void);
void configEINT3(void);
void configSystick(void);
uint8_t getDutyCycle(void);

uint8_t duty = 0;
uint8_t last_duty = 0xFF;
uint8_t enable = 1;
uint32_t periodo_ms = 1000 / PWM_FREQ_HZ;
uint32_t ms_bajo = 1000 / PWM_FREQ_HZ;
uint32_t contadorST = 1000 / PWM_FREQ_HZ;
uint32_t ms_alto = 0;


int main(void) {
    SystemInit();
    configGPIO();
    configGPIOInterrupt();
    configEINT3();
	configSystick();
	NVIC_EnableIRQ(SysTick_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

    while (1) {
		__WFI(); // Espera a la siguiente interrupción
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

void configGPIOInterrupt(void) {
    // Configura interrupciones por flancos P0.[3:0]
	// Falling edge
	LPC_GPIOINT->IO0IntEnF |= GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3; // 0xF
	// Risig edge
	LPC_GPIOINT->IO0IntEnR |= GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3; // 0xF
}

void configEINT3(void) {
    // Configura EINT3 (pulsador) como interrupción
    LPC_PINCON->PINSEL4 &= ~(0b11 << 26); 	// Limpia bits 26 y 27
	LPC_PINCON->PINSEL4 |= (0b01 << 26); 	// P2.13 como EINT3
    LPC_SC->EXTMODE |= (1 << 3); 			// EINT3 por flanco
    LPC_SC->EXTPOLAR |= (1 << 3); 			// EINT3 por flanco de subida
}

void configSystick(void) {
    // Configura el Systick para generar interrupciones cada 1 ms
    SysTick->LOAD = (SystemCoreClock / 1000) - 1; // Carga el valor para 1 ms
    SysTick->VAL = 0; // Limpia el valor actual
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // Fuente de reloj: CPU
                    SysTick_CTRL_TICKINT_Msk   | // Habilita interrupción
                    SysTick_CTRL_ENABLE_Msk;     // Habilita el Systick
}

uint8_t getDutyCycle(void) {
    uint8_t value = LPC_GPIO0->FIOPIN & 0x0F;
    if (value > PWM_MAX_DUTY) {
        value = PWM_MAX_DUTY;
    }
    return value;
}

void EINT3_IRQHandler(void) {
    if (LPC_SC->EXTINT & (1<<3)) { // Interrupción por EINT3
        if(enable){
		    LPC_GPIO0->FIOCLR |= PWM_PIN_MASK; 	//Turn off PWM pin
		    SysTick->CTRL &= ~(1<<1); 		//SysTick interrupts disabled
	    }
	    else{
		    SysTick->CTRL |= (1<<1); 		//SysTick interrupts enabled
	    }
		enable = !enable; // Cambia el estado de enable
        LPC_SC->EXTINT |= (1<<3); // Limpia la bandera de interrupción
    }
    else {
	    // Maneja las interrupciones por flanco en P0.[3:0]
        duty = getDutyCycle();
        if (LPC_GPIOINT->IO0IntStatR & (GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3)) {
            LPC_GPIOINT->IO0IntClr |= (GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3);
        }
        if (LPC_GPIOINT->IO0IntStatF & (GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3)) {
            LPC_GPIOINT->IO0IntClr |= (GPIO0_BIT0 | GPIO0_BIT1 | GPIO0_BIT2 | GPIO0_BIT3);
        }
	}
}
void EINT3_IRQHandler(void) {
    // Chequea EINT3
    if (LPC_SC->EXTINT & (1<<3)) {
        enable = !enable;
        if (!enable) {
            LPC_GPIO0->FIOCLR = PWM_PIN_MASK;
        }
        LPC_SC->EXTINT |= (1<<3); // limpia flag
    }
    // Chequea cambios en P0.[3:0]
    if (LPC_GPIOINT->IO0IntStatR & 0x0F || LPC_GPIOINT->IO0IntStatF & 0x0F) {
        duty = getDutyCycle();
        LPC_GPIOINT->IO0IntClr = 0x0F; // limpia flags de P0.0–P0.3
    }
}


void SysTick_Handler(void) {
    if(enable) {
        contadorST--;
        if(contadorST > ms_bajo) {
            LPC_GPIO0->FIOSET = PWM_PIN_MASK;
        }
        else if(contadorST > 0) {
            LPC_GPIO0->FIOCLR = PWM_PIN_MASK;
        }
        else if(contadorST <= 0) {
            // Actualiza tiempos y reinicia contador
            ms_alto = (periodo_ms * duty) / PWM_MAX_DUTY;
            ms_bajo = periodo_ms - ms_alto;
            contadorST = periodo_ms;
        }
    }
    else {
        LPC_GPIO0->FIOCLR = PWM_PIN_MASK;
    }
}