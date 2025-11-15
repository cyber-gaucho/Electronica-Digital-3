#include "buttons.h"

volatile uint8_t action = 0;  // Button action from ISR

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
}

void buttons_command(){
    if(NewState == ENABLE){
        NVIC_EnableIRQ(EINT3_IRQn);
    }else{
        NVIC_DisableIRQ(EINT3_IRQn);
    }
}


void buttons_handle_action(void){
    // static uint32_t lastActionTime = 0;
    if (action == 0) return;

    // if ((SysTick->VAL - lastActionTime) < 150) return;
    // lastActionTime = SysTick->VAL;

    switch (action) {
        case 1: // UP
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(5, 1); lcd_print("UP BUTTON");
            break;
        case 2: // DOWN
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("DOWN BUTTON");
            break;
        case 3: // RIGHT
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("RIGHT BUTTON");
            break;
        case 4: // LEFT
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("LEFT BUTTON");
            break;
        case 5: // SAVE
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("SAVE BUTTON");
            break;
        case 6: // SEND
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("SEND BUTTON");
            break;
        case 7: // ID
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("ID BUTTON");
            break;
    }
    action = 0;
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