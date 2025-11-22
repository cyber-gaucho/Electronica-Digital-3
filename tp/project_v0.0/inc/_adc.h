#ifndef _ADC_H
#define _ADC_H

#define ADC_CHANNEL 0
#define ADC_RATE 200000
#define ADC_TIMER_MATCH_VALUE 100

/*********************************************************************//**
 * @brief        Configura el ADC en el pin P0.23 para modo burst
 * @details      Inicializa el pin como entrada analógica, configura el ADC
 *               a 200 kHz y activa burst.
 * @param[in]    Ninguno
 * @return       None
 **********************************************************************/
void adc_init(void);

#endif
