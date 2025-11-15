#include "utils.h"

void LED_init(void){
    PINSEL_CFG_Type cfgLED = {0};
    cfgLED.Funcnum = PINSEL_FUNC_0; // GPIO
    cfgLED.Pinmode = PINSEL_PINMODE_TRISTATE;
    cfgLED.OpenDrain = PINSEL_PINMODE_NORMAL;
    cfgLED.Portnum = 0;
    cfgLED.Pinnum = LED_RED_PIN;
    PINSEL_ConfigPin(&cfgLED);

    cfgLED.Portnum = 3;
    cfgLED.Pinnum = LED_BLUE_PIN;
    PINSEL_ConfigPin(&cfgLED);

    cfgLED.Portnum = 3;
    cfgLED.Pinnum = LED_GREEN_PIN;
    PINSEL_ConfigPin(&cfgLED);

    GPIO_SetDir(0,1<<LED_RED_PIN,1);
    GPIO_SetDir(3, 1<<LED_BLUE_PIN|1<<LED_GREEN_PIN,1);

    GPIO_SetValue(0, 1<<LED_RED_PIN);
    GPIO_SetValue(3,1<<LED_BLUE_PIN|1<<LED_GREEN_PIN);
}

void LED_set(uint8_t r, uint8_t g, uint8_t b){
    if(r) GPIO_ClearValue(0, 1<<LED_RED_PIN); else GPIO_SetValue(0, 1<<LED_RED_PIN);
    if(g) GPIO_ClearValue(3, 1<<LED_GREEN_PIN); else GPIO_SetValue(3, 1<<LED_GREEN_PIN);
    if(b) GPIO_ClearValue(3, 1<<LED_BLUE_PIN); else GPIO_SetValue(3, 1<<LED_BLUE_PIN);
}

void delay(){
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    for (volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();
    }
}
