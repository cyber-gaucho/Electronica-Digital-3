#ifndef BUTTONS_H
#define BUTTONS_H

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

#define     BTN_PORT        2
/**
 * @enum BTN_OPT
 * @brief Enumeración de pines asociados a botones de control.
 *
 * Esta enumeración define los identificadores de los pines utilizados para los botones.
 * El valor BTN_ID_PIN debe permanecer como el último elemento ya que se usa en un for loop.
 *
 * Nota: El pin P2.9 no está disponible en la placa.
 */
 enum {
    BTN_UP_PIN = 0,
    BTN_DOWN_PIN, 
    BTN_RIGHT_PIN,
    BTN_LEFT_PIN,
    BTN_SAVE_PIN,
    BTN_SEND_PIN,
    BTN_ID_PIN      // Dejar ID ultimo en este enum
} BTN_OPT;

void buttons_init();

void buttons_command(FunctionalState NewState);

void buttons_handle_action();

#endif /* BUTTONS_H */