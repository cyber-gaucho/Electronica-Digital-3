/**
 * @file    25_09_11_Timers.c
 * @brief   Ejemplo de uso de timers: PWM y Capture
 * 
 * Genera una señal PWM en P1.0 con periodo de 1ms y duty cíclico aleatorio entre 0 y 100%
 * Cada 1s actualiza el duty con un valor pseudoaleatorio.
 * Se utiliza el Timer0 con dos Match Register (MR1 para periodo, MR0 para duty).
 * Mediante un contador en MR1 se actualiza el duty cada 1s.
 */
#include "E:\Electronica-Digital-3\common\cmsis\CMSISv2p00_LPC17xx-TRUJILLO\inc\LPC17xx.h"
#include <stdio.h>

#define BIT_MASK(x)     (1 << x)
#define BITS_MASK(x,s)  (((1 << x) - 1) << s)
#define PWM_PIN         0       // P1.0 -> PWM
#define PWM_PORT        LPC_GPIO1
#define PWM_PINSEL      PINSEL2

#define BANK0_ADDR 0x2007C000
volatile uint32_t *capture_buffer = (uint32_t *)BANK0_ADDR;
volatile uint32_t capture_index = 0;

volatile uint32_t duty_ticks = 0;
volatile uint32_t period_ticks = 25000; // 1 kHz con PCLK=25MHz

void TIMER0_IRQHandler(void) {
    static uint32_t counter = 0;
    if (LPC_TIM0->IR & (1 << 0)) { // Match0 → duty
        PWM_PORT->FIOCLR = (1 << PWM_PIN); // P1.0 en bajo
        LPC_TIM0->IR = (1 << 0);      // clear flag
    }
    if (LPC_TIM0->IR & (1 << 1)) { // Match1 → periodo
        PWM_PORT->FIOSET = (1 << PWM_PIN); // P1.0 en alto
        LPC_TIM0->IR = (1 << 1);      // clear flag
        counter++;
        if(counter >= 1000) {
            update_duty();
            counter = 0;
        }
    }
}

int rand_duty(void) {
    // Generar pseudo-random simple
    static uint32_t seed = 12345;
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return (seed % 101); // 0–100
}

void update_duty(void) {
    int duty = rand_duty();
    duty_ticks = (period_ticks * duty) / 100;
    LPC_TIM0->MR1 = duty_ticks;
    LPC_TIM0->LER = (1 << 1); // latch MR1
}

int main(void) {

	//Configurar P1.0 como puerto y salida
	LPC_PINCON->PWM_PINSEL &= ~(0x3 << (PWM_PIN%16)*2); // Limpio bits dejandolos en 00 (funcion GPIO), pinmode no me importa

	//TIMER0 como generador de pwm
	LPC_SC->PCONP |= (1 << 1); //Con 1 energizo el periferico
	LPC_SC->PCLKSEL0 &= ~(0x3 << 2); //clk0 = cclk/4 -> 25 MHz
	LPC_TIM0->PR = 0;	   //No preescaler
	LPC_TIM0->MR0 = period_ticks;
	LPC_TIM1->MR1 = period_ticks/2; //duty de 50% al inicio 

	LPC_TIM0->MCR = (1 <<0) | (1 << 1) | (1 << 3); //INTERRUPT Y RESET TM0, INT EN TM1, puedo acceder desde cualquier tim?

    NVIC_EnableIRQ(TIMER0_IRQn);
    LPC_TIM0->TCR = 1; // enable timer

	// //TIMER1 como capture
	// LPC_SC->PCONP |= (1 << 2);
	// LPC_SC->PCLKSEL0 &= ~(0x3 << 4);
	// LPC_PINCON->PINSEL3 &= ~(0x3 << 4);
	// LPC_PINCON->PINSEL3 |=(0x3 << 4); //P1.18 como cap1.0
	// LPC_TIM1->CCR = (1 << 0) | (1 << 2) | (1 << 3); // Capture on rising edge + falling + interrupt

    NVIC_EnableIRQ(TIMER1_IRQn);
    LPC_TIM1->TCR = 1; // enable timer

    while(1){
    	static uint32_t last = 0;
    	if((LPC_TIM0->TC - last) >= 25000000){
    		last = LPC_TIM0->TC;
    		update_duty();
    	}
    }
}