#ifndef _ADC_H
#define _ADC_H

#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#define ADC_CHANNEL 0
#define ADC_RATE 200000
#define ADC_TIMER_MATCH_VALUE 100

extern uint16_t kilos;

/*********************************************************************//**
 * @brief        Configura el ADC en el pin P0.23 para modo normal, habilita
 *               la interrupción en el canal 0 y el timer0 para generar una 
 *               interrupción cada 100ms.
 * @details      Inicializa el pin como entrada analógica, configura el ADC
 *               a 200 kHz, desactiva burst, selecciona disparo por flanco
 *               descendente, habilita el canal 0 y su interrupción.
 * @param[in]    Ninguno
 * @return       None
 **********************************************************************/
void adc_init(void);


/*********************************************************************//**
 * @brief        Comando para iniciar la conversión del ADC
 * @details      Inicia la conversión del ADC en modo MAT01 y el timer0 para 
 *               generar una interrupción cada 100ms. 
 * @param[in]    NewState: ENABLE para iniciar la conversión del ADC
                           DISABLE para detenerla y reiniciar el timer0.
 * @return       None
 **********************************************************************/
void adc_comand(FunctionalState NewState);

/*********************************************************************//**
 * @brief Handler de la interrupción del ADC
 * @details Procesa las interrupciones del ADC, lee el valor del canal 0,
 *          actualiza la variable global kilos y controla el LED según el valor leído
 * @param[in]    Ninguno
 * @return       None
 **********************************************************************/
void ADC_IRQHandler(void);

#endif
