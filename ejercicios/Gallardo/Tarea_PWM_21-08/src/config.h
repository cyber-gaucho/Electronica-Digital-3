/*
 * config.h
 *
 *  Created on: 23 ago 2025
 *      Author: Usuario
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "LPC17xx.h"
#include <stdio.h>

#define BIT_MASK(x)     (1 << x)
#define BITS_MASK(x,s)  (((1 << x) - 1) << s)

#define PWM_PIN_NUMBER 	22
#define PWM_MAX_DUTY 	15
#define PWM_FREQ_HZ 	20

#define GPIO0_BIT0	  BIT_MASK(0) 		// Pin 0.0
#define GPIO0_BIT0_DB BITS_MASK(2, 0) 	// Pin 0.0 (doble bit)
#define GPIO0_BIT1 	  BIT_MASK(1) 		// Pin 0.1
#define GPIO0_BIT1_DB BITS_MASK(2, 2) 	// Pin 0.1 (doble bit)
#define GPIO0_BIT2 	  BIT_MASK(2) 		// Pin 0.2
#define GPIO0_BIT2_DB BITS_MASK(2, 4) 	// Pin 0.2 (doble bit)
#define GPIO0_BIT3 	  BIT_MASK(3) 		// Pin 0.3
#define GPIO0_BIT3_DB BITS_MASK(2, 6) 	// Pin 0.3 (doble bit)

#define PWM_PIN_MASK 	BIT_MASK(PWM_PIN_NUMBER)
#define PWM_PIN_MASK_DB BITS_MASK(2, (PWM_PIN_NUMBER-16)*2)


#endif /* CONFIG_H_ */
