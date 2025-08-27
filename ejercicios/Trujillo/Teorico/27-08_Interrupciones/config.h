/*
 * config.h
 *
 *  Created on: 27 ago 2025
 *      Author: Usuario
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#define BIT_MASK(x)     (1 << x)
#define BITS_MASK(x,s)  (((1 << x) - 1) << s)

#define LED_PIN     22        // P0.22
#define LED_MASK    (1 << LED_PIN)
#define LED_MASK_PINSEL BITS_MASK(2, (LED_PIN-16)*2)

#define GPIO_INT_PIN   0      // P2.0
#define GPIO_INT_MASK  (1 << GPIO_INT_PIN)
#define GPIO_INT_PINSEL BITS_MASK(2, (GPIO_INT_PIN*2))

#define EINT2_PIN   12      // P2.12
#define EINT2_MASK  (1 << EINT2_PIN)
#define EINT2_PINSEL BITS_MASK(2, (EINT2_PIN*2

#endif /* CONFIG_H_ */
