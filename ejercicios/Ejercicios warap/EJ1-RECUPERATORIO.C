/**
 * Ejercicio N 1: (40%)
Por un pin del ADC del microcontrolador LPC1769 ingresa una tensión de rango dinámico 0 a 3,3[v] 
proveniente de un sensor de temperatura. Debido a la baja tasa de variación de la señal,
se pide tomar una muestra cada 30[s]. Pasados los 2[min] se debe promediar las últimas 
 4 muestras y en función de este valor, tomar una decisión sobre una salida digital de la placa:
● Si el valor es <1 [V] colocar la salida en 0 (0[V]).
● Si el valor es >= 1[V] y <=2[V] modular una señal PWM con un Ciclo de trabajo que va desde 
el 50% hasta el 90% proporcional al valor de tensión, con un periodo de 20[KHz].
● Si el valor es > 2[V] colocar la salida en 1 (3,3[V]).
 */

#include "LPC17xx.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "LPC17xx.h"
#include <stdint.h>

#define PWM_PORT 0
#define PWM_PORT_POINTER LPC_GPIO0
#define PWM_PIN 9 // P0.9

// uint16_t adcVal = 0;
// uint16_t adcAvg = 0;


void configADC(void);
void configTimer0(void);   // Para leer ADC - prioridad baja
void configTimer1(void);   // Para PWM - prioridad alta
void configPCB(void);

int main() {
    SystemInit();
    configPCB();
    configTimer0();
    configADC();
    configTimer1();
    while(1) {}
    return 0;
}

void configADC(void) {
    ADC_Init(LPC_ADC, 200000);          // 200 kHz
    ADC_ChannelCmd(LPC_ADC, 0, ENABLE); // HABILITO CANAL 0
    ADC_BurstCmd(LPC_ADC, DISABLE);
    ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);  // INICIO CON TIMER0 MATCH1
    ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
    ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE); // HABILITO INT CANAL 0
    NVIC_EnableIRQ(ADC_IRQn);
}

void configTimer0(void) {
    TIM_TIMERCFG_Type timer0 = {0};
    TIM_MATCHCFG_Type match1 = {0};

    timer0.PrescaleOption = TIM_PRESCALE_USVAL;
    timer0.PrescaleValue = 1000000;   // 1 s 
    
    match1.MatchChannel = 1;
    match1.IntOnMatch = DISABLE;
    match1.StopOnMatch = DISABLE;
    match1.ResetOnMatch = ENABLE;
    match1.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    match1.MatchValue = 15; // flanco cada 30 segundos

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timer0);
    TIM_ConfigMatch(LPC_TIM0, &match1);
    // NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

void configTimer1() {
    TIM_TIMERCFG_Type timer1 = {0};
    TIM_MATCHCFG_Type match0={0};
    TIM_MATCHCFG_Type match1={0};

    timer1.PrescaleOption = TIM_PRESCALE_USVAL;
    timer1.PrescaleValue = 1;   // 1 us 
    
    match0.MatchChannel = 0;
    match0.IntOnMatch = ENABLE;
    match0.StopOnMatch = DISABLE;
    match0.ResetOnMatch = DISABLE;
    match0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    match0.MatchValue = 25; // entre 50 y 90% del periodo 25 - 45 us

    match1.MatchChannel = 1;
    match1.IntOnMatch = ENABLE;
    match1.StopOnMatch = DISABLE;
    match1.ResetOnMatch = ENABLE;
    match1.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    match1.MatchValue = 50;  // 50 us (20 kHz)

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer1);
    TIM_ConfigMatch(LPC_TIM1, &match0);
    TIM_ConfigMatch(LPC_TIM1, &match1);
    NVIC_EnableIRQ(TIMER1_IRQn);
    TIM_Cmd(LPC_TIM1, ENABLE);
}

void configPCB(void) {
    // Configuro pin ADC P0.23
    PINSEL_CFG_Type cfgPin;
    cfgPin.Portnum = PINSEL_PORT_0;
    cfgPin.Pinnum = PINSEL_PIN_23;
    cfgPin.Funcnum = PINSEL_FUNC_1;
    cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;
    cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&cfgPin);
    
    //Configuro pin de Salida
    PINSEL_CFG_Type cfgPin;
    cfgPin.Portnum = PWM_PORT;
    cfgPin.Pinnum = PWM_PIN;
    cfgPin.Funcnum = PINSEL_FUNC_0;             // Función GPIO
    cfgPin.Pinmode = PINSEL_PINMODE_TRISTATE;   // sin pull-up ni pull-down
    cfgPin.OpenDrain = PINSEL_PINMODE_NORMAL;
    PINSEL_ConfigPin(&cfgPin);

    GPIO_SetDir(PWM_PORT, (1 << PWM_PIN), 1);       // Salida
    GPIO_ClearValue(PWM_PORT, (1 << PWM_PIN));      // Salida en 0
}

void ADC_IRQHandler(void) {
    static uint8_t cont_30s = 0;
    static uint16_t adcVal = 0;
    static uint16_t adcBuffer[4] = {};
    static float adcAvg = 0;

    if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)) {
        adcVal = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        adcBuffer[cont_30s] = adcVal;
        cont_30s++;
        if(cont_30s == 4) {
            adcAvg = (float)(adcBuffer[0] + adcBuffer[1] + adcBuffer[2] + adcBuffer[3]) / 4.0f;
            cont_30s = 0;
            adcBuffer[0] = adcBuffer[1] = adcBuffer[2] = adcBuffer[3] = 0;
        }
        
        if(adcAvg < 1241){                      // 1V = 1241
            TIM_Cmd(LPC_TIM1, DISABLE);         // Apagar PWM
            PWM_PORT_POINTER->FIOCLR = (1 << PWM_PIN);  // salida en 0v
        }
        else if(adcAvg > 2482){                 // 2V = 2482
            TIM_Cmd(LPC_TIM1, DISABLE);         // Apagar PWM
            PWM_PORT_POINTER->FIOSET = (1 << PWM_PIN);  // salida en 3.3v
        }
        else {
            uint8_t match = 0;
            match = (uint8_t)(25 + 20 * ((adcAvg / 1241) - 1));     // entre 50 y 90% (25 - 45 us)
            TIM_UpdateMatchValue(LPC_TIM1, 0, match);    // Actualizo Match 0
            TIM_Cmd(LPC_TIM1, ENABLE);                  // Encender PWM
        }

        // Flag de interrupción se limpia automáticamente al leer el dato
    }
}

void TIMER1_IRQHandler(void) {
    if(LPC_TIM1->IR & 1) {
        LPC_TIM1->IR = 1;
        PWM_PORT_POINTER->FIOCLR = (1 << PWM_PIN);
    }
    else if(LPC_TIM1->IR & 2) {
        LPC_TIM1->IR = 2;
        PWM_PORT_POINTER->FIOSET = (1 << PWM_PIN);
    }

    // if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)){
    //     PWM_PORT_POINTER->FIOCLR = (1 << PWM_PIN);
    //     TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    // }
    // else if (TIM_GetIntStatus(LPC_TIM1, TIM_MR1_INT)){
    //     PWM_PORT_POINTER->FIOSET = (1 << PWM_PIN);
    //     TIM_ClearIntPending(LPC_TIM1, TIM_MR1_INT);
    // }
}