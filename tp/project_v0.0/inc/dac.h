#ifndef DAC_H
#define DAC_H

#include <stdint.h>

#define SAMPLES_AMOUNT      256     // Cantidad de muestras de la tabla de seno
#define SINE_FREQ_IN_HZ     440     // Frecuencia target en Hz (tono La4)
#define PCLK_DAC_IN_MHZ     25      // Frecuencia de clock periférico del DAC (MHz)


extern uint32_t sine_bank[SAMPLES_AMOUNT];
extern volatile uint8_t delay_flag;

/**
 * @brief Inicializa el DAC y su pin de salida.
 * 
 * Configura P0.26 como salida de DAC, activa modo de contador y DMA
 * en el convertidor digital analógico.
 */
void dac_init(void);            

/**
 * @brief Configura canal de DMA para el DAC usando una lista ligada.
 * 
 * Prepara una transferencia circular de la tabla de muestras al registro del DAC,
 * utilizando el canal 0 del DMA.
 */
void configDMA_DAC_Channel(void);

/**
 * @brief Inicializa el timer 2 (TIM2) para generar un retardo específico en ms.
 * 
 * @param ms Retardo en milisegundos.
 */
void TIM2_init(uint32_t ms);

/**
 * @brief Retardo bloqueante utilizando TIM2 (milisegundos).
 *
 * Espera activa utilizando un flag modificado por la ISR del TIMER2.
 *
 * @param ms Tiempo a esperar en milisegundos.
 */
void delayTIM2(uint32_t ms);

/**
 * @brief Genera un tono a una frecuencia dada por un cierto tiempo.
 * 
 * @param frec_Hz Frecuencia en Hz de la onda senoidal.
 * @param ms Duración en milisegundos del tono generado.
 */
void generateTone(uint16_t frec_Hz, uint32_t ms);

/**
 * @brief Configuración de los pines de los LEDs
 * @details Inicializa los pines de los LEDs como salidas
 * @param[in] Ninguno
 * @return None
 */
 void LED_init(void);

 /**
  * @brief Enciende o apaga los LEDs
  * @param[in] r: Valor del LED rojo
  * @param[in] g: Valor del LED verde
  * @param[in] b: Valor del LED azul
  * @note Enciende el LED con VALOR 1 y apaga con VALOR 0
  * @return None
  */
 void LED_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Función principal.
 * 
 * Inicializa el sistema y genera un tono de 440 Hz por 3 segundos,
 * luego espera 10 segundos y repite el proceso.
 */

#endif /* DAC_H_ */