#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_libcfg_default.h"
#include "lpc17xx_nvic.h"
#include "lpc17xx_adc.h"

#define PORT_0 (uint8_t) 0
#define PIN_0 ((uint32_t)(1 << 0))
#define PIN_1 ((uint32_t)(1 << 1))
#define PIN_2 ((uint32_t)(1 << 2))

void cfgTimer(void);
void cfgPCB(void);
void cfgADC(void);
void prenderAmarillo(void);
void prenderRojo(void);
void prenderVerde(void);
void apagarRojo(void);

uint16_t iRojo = 0;
uint32_t conversion;

int main() {
    cfgPCB();
    cfgTimer();
    cfgADC();

    while(1){}

    return 0;
}

void cfgTimer(void){
    TIM_TIMERCFG_Type cfgTimerMode;
    cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
    cfgTimerMode.PrescaleValue = 1000;

    TIM_MATCHCFG_Type cfgTimerMatch;
    cfgTimerMatch.MatchValue = 49;
    cfgTimerMatch.MatchChannel = 1;
    cfgTimerMatch.IntOnMatch = DISABLE;
    cfgTimerMatch.ResetOnMatch = ENABLE;
    cfgTimerMatch.StopOnMatch = DISABLE;
    cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
    TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

void cfgPCB(void){
    PINSEL_CFG_Type pinADC = {PINSEL_PORT_0, PINSEL_PIN_23, PINSEL_FUNC_1, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_NORMAL};
    PINSEL_CFG_Type pinRojo = {PINSEL_PORT_0, PINSEL_PIN_0, PINSEL_FUNC_0, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_NORMAL};
    GPIO_SetDir(PORT_0, PIN_0, OUTPUT);
    PINSEL_CFG_Type pinVerde = {PINSEL_PORT_0, PINSEL_PIN_1, PINSEL_FUNC_0, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_NORMAL};
    GPIO_SetDir(PORT_0, PIN_1, OUTPUT);
    PINSEL_CFG_Type pinAmarillo = {PINSEL_PORT_0, PINSEL_PIN_2, PINSEL_FUNC_0, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_NORMAL};
    GPIO_SetDir(PORT_0, PIN_2, OUTPUT);
    PINSEL_CFG_Type pinMAT01 = {PINSEL_PORT_1, PINSEL_PIN_29, PINSEL_FUNC_3, PINSEL_PINMODE_TRISTATE, PINSEL_PINMODE_NORMAL};
    
    PINSEL_ConfigPin(&pinADC);
    PINSEL_ConfigPin(&pinAmarillo);
    PINSEL_ConfigPin(&pinRojo);
    PINSEL_ConfigPin(&pinVerde);
    PINSEL_ConfigPin(&pinMAT01);
}

void cfgADC(void){
    ADC_Init(LPC_ADC, 200000);
    ADC_BurstCmd(LPC_ADC, DISABLE);
    ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
    ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
    ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);
    NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_IRQHandler(){
    while(!ADC_ChanelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){}
    conversion = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
    if(conversion >= 2498){
        iRojo++;
        apagarAmarillo();
        apagarVerde();

    }
    else if(conversion >= 1679 && conversion < 2498){
        prenderAmarillo();
        iRojo = 0;
        apagarRojo();
        apagarVerde();
    }
    else{
        prenderVerde();
        iRojo = 0;
        apagarRojo();
        apagarAmarillo();
    }

    if(iRojo >= 10){
        prenderRojo();
    }
}

void prenderAmarillo(){
    GPIO_SetValue(PORT_0, PIN_2);
}

void prenderRojo(){
    GPIO_SetValue(PORT_0, PIN_0);
}

void prenderVerde(){
    GPIO_SetValue(PORT_0, PIN_1);
}

void apagarRojo(){
    GPIO_ClearValue(PORT_0, PIN_0);
}

void apagarVerde(){
    GPIO_ClearValue(PORT_0, PIN_1);
}

void apagarAmarillo(){
    GPIO_ClearValue(PORT_0, PIN_2);
}