#ifndef UTILS_H
#define UTILS_H

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

/* LED Pin Configuration */
#define LED_RED_PIN     22  // P0.22
#define LED_BLUE_PIN    26  // P3.26
#define LED_GREEN_PIN   25  // P3.25

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
 */
void delay();



#endif