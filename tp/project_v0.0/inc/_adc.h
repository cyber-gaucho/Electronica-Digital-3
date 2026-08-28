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

/*********************************************************************//**
 * @brief        Convierte ADC value a kilos
 * @details      This function takes the ADC value from channel 0 and
 *               converts it to kilos. It takes the ADC value and
 *               multiplies it by 999, then divides the result by
 *               4095. The result is returned as a 16-bit
 *               unsigned integer.
 * @return       Kilos value [0 - 999]
 **********************************************************************/
uint16_t adc_getkilos(void);

#endif /* _ADC_H */