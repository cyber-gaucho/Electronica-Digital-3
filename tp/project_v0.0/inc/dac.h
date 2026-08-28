#ifndef DAC_H
#define DAC_H

#include <stdint.h>

#define SAMPLES_AMOUNT      256     // Cantidad de muestras de la tabla de seno
#define SINE_FREQ_IN_HZ     440     // Frecuencia target en Hz (tono La4)
#define PCLK_DAC_IN_MHZ     25      // Frecuencia de clock periférico del DAC (MHz)

/**
 * @brief Inicializa el DAC y su pin de salida.
 *
 * Configura P0.26 como salida de DAC, activa modo de contador y DMA
 * en el convertidor digital analógico.
 */
void dac_init(void);

/**
 * @brief Genera un tono a una frecuencia dada por un cierto tiempo.
 *
 * @param frec_Hz Frecuencia en Hz de la onda senoidal.
 * @param ms Duración en milisegundos del tono generado.
 */
void generateTone(uint16_t frec_Hz, uint32_t ms);

#endif /* DAC_H_ */