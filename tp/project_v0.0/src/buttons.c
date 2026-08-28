#include "buttons.h"

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

void buttons_init(){
    PINSEL_CFG_Type cfgButtons = {0};
    cfgButtons.Portnum = BTN_PORT;
    cfgButtons.Funcnum = PINSEL_FUNC_0; // GPIO
    cfgButtons.Pinmode = PINSEL_PINMODE_PULLUP;
    cfgButtons.OpenDrain = PINSEL_PINMODE_NORMAL;
    for(uint8_t i = 0; i <= BTN_ID_PIN; i++){
        cfgButtons.Pinnum = i;
        PINSEL_ConfigPin(&cfgButtons);
    }
    LPC_GPIO2->FIODIR &= ~((1 << BTN_UP_PIN) | (1 << BTN_DOWN_PIN) | (1 << BTN_RIGHT_PIN) |
                       (1 << BTN_LEFT_PIN) | (1 << BTN_SAVE_PIN) | (1 << BTN_SEND_PIN) |
                       (1 << BTN_ID_PIN));

    GPIO_ClearInt(2,0xFFFF);  // Limpia cualquier interrupción previa
    GPIO_IntCmd(2,((1 << BTN_UP_PIN) | (1 << BTN_DOWN_PIN) | (1 << BTN_RIGHT_PIN) |
                    (1 << BTN_LEFT_PIN) | (1 << BTN_SAVE_PIN) | (1 << BTN_SEND_PIN) |
                    (1 << BTN_ID_PIN)),1);

    LPC_GPIOINT->IO2IntClr = 0xFFFF;

    NVIC_EnableIRQ(EINT3_IRQn);
}

/**
 * @brief Handler de las interrupciones por GPIO para los botones
 */
 void EINT3_IRQHandler(void) {
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_UP_PIN)) { action = 1; LPC_GPIOINT->IO2IntClr = (1 << BTN_UP_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_DOWN_PIN)) { action = 2; LPC_GPIOINT->IO2IntClr = (1 << BTN_DOWN_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_RIGHT_PIN)) { action = 3; LPC_GPIOINT->IO2IntClr = (1 << BTN_RIGHT_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_LEFT_PIN)) { action = 4; LPC_GPIOINT->IO2IntClr = (1 << BTN_LEFT_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_SAVE_PIN)) { action = 5; LPC_GPIOINT->IO2IntClr = (1 << BTN_SAVE_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_SEND_PIN)) { action = 6; LPC_GPIOINT->IO2IntClr = (1 << BTN_SEND_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_ID_PIN)) { action = 7; LPC_GPIOINT->IO2IntClr = (1 << BTN_ID_PIN); }
}