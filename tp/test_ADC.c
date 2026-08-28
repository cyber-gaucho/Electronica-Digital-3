/**
 * @file test_ADC.c
 * @brief Test routines for the ADC (Analog-to-Digital Converter) module.
 *
 * This file contains functions and test cases to verify the correct operation
 * of the ADC hardware and its integration with the system.
 *
 * @author
 * @date
 */

#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "E:\Electronica-Digital-3\tp\librerias\LiquidCrystal_I2C_LPC\inc\LiquidCrystal_I2C_LPC.h"
#include <stdlib.h>

#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0
#define 	RED_LED      	(1<<22)
#define     FILA_KILOS      3

char fila3[12]="   Kilos    ";

void cfgI2C0(void);
void test_I2C(void);

/**
* @brief Configuración del LED integrado (P0.22)
*/
void cfgGPIO();

/**
* @brief Configuración del ADC (P0.23)
*/
void cfgADC();

void cfgTimer();

void lcd_update_kilos(uint16_t value);

/**
* @brief Función principal
*/
int main(){
	cfgI2C0();        // Inicializa el periférico I2C
    lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD
    lcd_begin(LCD_I2C_P, 20, 4);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(0, 0); lcd_print("      Grupo 1");
    lcd_setCursor(0, 1); lcd_print("  Garcia Lautaro M ");
    lcd_setCursor(0, 2); lcd_print(" Renaudo G Valentino");
    lcd_setCursor(0, 3); lcd_print(fila3);
	cfgGPIO();
	cfgADC();
    cfgTimer();
	while(1);
    return 0;
}

void cfgI2C0(){
	// Configurar pines P0.27 (SDA0) y P0.28 (SCL0) función 1
	LPC_PINCON->PINSEL1 &= ~((3 << 22) | (3 << 24));
	LPC_PINCON->PINSEL1 |=  ((1 << 22) | (1 << 24));

	// Modo estándar (100–400 kHz)
	LPC_PINCON->I2CPADCFG = 0x00;

	// Inicializar periférico
	I2C_Init(LCD_I2C_P, 100000);

	/* Enable Slave I2C operation */
	I2C_Cmd(LCD_I2C_P, ENABLE);
}

void test_I2C(){
	I2C_M_SETUP_Type test_cfg;
	uint8_t cero = 0x00;
	test_cfg.sl_addr7bit = 0x27;
	test_cfg.tx_data = &cero;
	test_cfg.tx_length = 1;
	test_cfg.rx_data = NULL;
	test_cfg.rx_length = 0;
	test_cfg.retransmissions_max = 3;

	I2C_MasterTransferData(LPC_I2C0, &test_cfg, I2C_TRANSFER_POLLING);
}

void cfgGPIO(){
	PINSEL_CFG_Type pinGPIO = {0};
	pinGPIO.Portnum = 0;
	pinGPIO.Pinnum = 22;
	pinGPIO.Funcnum = 0;
	pinGPIO.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinGPIO.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinGPIO);
    GPIO_SetDir(0,RED_LED, 1);
	LPC_GPIO0->FIOSET |= (RED_LED); // Apagar led rojo (activo bajo)
	// LPC_GPIO3->FIOSET |= (1<<25); // Apagar led verde (activo bajo)
	// LPC_GPIO3->FIOSET |= (1<<26); // Apagar led azul (activo bajo)
}

/**
* @brief Configuración del ADC (P0.23)
*/
void cfgADC(){
	PINSEL_CFG_Type pinADC = {0};
	pinADC.Portnum = 0;
	pinADC.Pinnum = 23;
	pinADC.Funcnum = 1;
	pinADC.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinADC.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinADC);

	ADC_Init(LPC_ADC, 20000);                           // ADC a 200kHz
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);     // Habilitar CH 0
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);       // Habilitar INT para canal 0
	NVIC_EnableIRQ(ADC_IRQn);                           // Habilitar INT en NVIC
    ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
}

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerMode.PrescaleValue = 1000;

	cfgTimerMatch.MatchChannel = 1;
	cfgTimerMatch.MatchValue = 100 - 1;
	cfgTimerMatch.IntOnMatch = DISABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

void lcd_update_kilos(uint16_t value){
    uint16_t kilos = (value * 999) / 4095;  // Mapeo entre 0 y 999
    char kilos_str[3];
    uitoa(kilos, kilos_str, 10);
    // uitoa(unsigned int value, char *vstring, unsigned int base)
	lcd_setCursor(12, FILA_KILOS);
	lcd_print(kilos_str);
	lcd_print(" Kg  ");
}

/**
* @brief Handler de la interrupción del ADC
*/
void ADC_IRQHandler(void){
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){
		uint16_t adcValue = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
		lcd_update_kilos(adcValue);
        if(adcValue == 4095) {
            LPC_GPIO0->FIOCLR |= (RED_LED); // Enciende LED rojo (activo bajo)
        } else {
            LPC_GPIO0->FIOSET |= (RED_LED); // Apagar LED rojo (activo bajo)
        }
		// ADC_ClearIntPending(LPC_ADC, ADC_ADINTEN0); // Clear the ADC interrupt flag
	}
}