/*
Ejercicio 1 - Parcial 2023
Demodulación PWM con Capture y DAC
- Entrada: Señal PWM por pin CAP0.0
- Salida: Tensión 0-2V por DAC proporcional al duty cycle
- Actualización cada 0.5s con promedio de 10 muestras
*/
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_dac.h"


#define MUESTRAS 10
#define VALOR_2V_DAC 620      // Valor DAC para 2V .  (2*1023)/3.3=620

volatile uint32_t capturaAnterior = 0;
volatile uint32_t periodo = 0;
volatile uint32_t CICLO_TRABAJO = 0;
volatile uint8_t flancoSubida = 1;
volatile uint32_t CAPTURAS_DE_CICLOS_TRABAJO[MUESTRAS];
volatile uint8_t INDICE_CAPTURAS = 0;

void configPIN(void);
void configTIMER0(void);
void configTIMER1(void);
void configDAC(void);

int main(void) 
    configPIN();
    configTIMER0();  // Para Capture
    configTIMER1();  // Para DAC
    configDAC();
    
    while(1) {
        __WFI();
    }
    return 0;
}

void configPIN(void) {
    PINSEL_CFG_Type pinCfg;
    
    // CAP0.0 en P1.26
    pinCfg.Portnum = PINSEL_PORT_1;
    pinCfg.Pinnum = PINSEL_PIN_26;
    pinCfg.Funcnum = PINSEL_FUNC_3;    // Función CAP0.0
    pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
    
    // DAC en P0.26
   /* PINSEL_CFG_Type dacPinCfg;
    dacPinCfg.Portnum = PINSEL_PORT_0;
    dacPinCfg.Pinnum = PINSEL_PIN_26;
    dacPinCfg.Funcnum = PINSEL_FUNC_2;  // AOUT
    dacPinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    */ // el dac_init() ya me configura el pin para dac
    PINSEL_ConfigPin(&pinCfg);
    //PINSEL_ConfigPin(&dacPinCfg);
}

void configTIMER0(void) {
    TIM_TIMERCFG_Type timerCfg;
    TIM_CAPTURECFG_Type captureCfg;
    
    //cada 1us 
    timerCfg.PrescaleOption = TIM_PRESCALE_USVAL;
    timerCfg.PrescaleValue = 1;
    
    // Capture 0
    captureCfg.CaptureChannel = 0;     
    captureCfg.RisingEdge = ENABLE;    
    captureCfg.FallingEdge = ENABLE;   
    captureCfg.IntOnCaption = ENABLE;  
    
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &timerCfg);
    TIM_ConfigCapture(LPC_TIM0, &captureCfg);
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Cmd(LPC_TIM0, ENABLE);
}

void configTIMER1(void) {
    TIM_TIMERCFG_Type timer1Cfg;
    TIM_MATCHCFG_Type match1Cfg;
    
    // Timer1 
    timer1Cfg.PrescaleOption = TIM_USVAL;
    timer1Cfg.PrescaleValue = 1;
    
    match1Cfg.MatchChannel = 0;
    match1Cfg.IntOnMatch = ENABLE;
    match1Cfg.ResetOnMatch = ENABLE;
    match1Cfg.StopOnMatch = DISABLE;
    match1Cfg.ExtMatchOutputType = TIM_NOTHING;
    match1Cfg.MatchValue = 500000;// 0.5s
    
    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &timer1Cfg);
    TIM_ConfigMatch(LPC_TIM1, &match1Cfg);
    NVIC_EnableIRQ(TIMER1_IRQn);
    TIM_Cmd(LPC_TIM1, ENABLE);
}

void configDAC(void) {
    DAC_Init();
    DAC_SetBias(DAC_700uA);  
}

void TIMER0_IRQHandler(void) {
    uint32_t capturaActual;
    if(TIM_GetIntCaptureStatus(LPC_TIM0, 0)) {
        capturaActual = TIM_GetCaptureValue(LPC_TIM0, 0);
        if(flancoSubida){
            periodo = capturaActual - capturaAnterior;
            flancoSubida = 0;
        } 
        else{
            CICLO_TRABAJO = capturaActual - capturaAnterior;
            // Calcular y guardar ciclo de trabajo en porcentaje
            CAPTURAS_DE_CICLOS_TRABAJO[INDICE_CAPTURAS] = (CICLO_TRABAJO * 100) / periodo;
            INDICE_CAPTURAS = (INDICE_CAPTURAS + 1) % MUESTRAS;
            flancoSubida = 1;
        }
        
        capturaAnterior = capturaActual;
        TIM_ClearIntCapturePending(LPC_TIM0, 0);
    }
}

void TIMER1_IRQHandler(void) {
    if(TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)) {
        // Calcular promedio de los últimos 10 valores
        uint32_t suma = 0;
        for(int i = 0; i < MUESTRAS; i++) {
            suma += CAPTURAS_DE_CICLOS_TRABAJO[i];
        }
        uint32_t promedio = suma / MUESTRAS;
        // Convertir promedio  a valor DAC entre 0-620 para 0-2V
        uint32_t valorDAC = (promedio * VALOR_2V_DAC) / 100;
        DAC_UpdateValue(valorDAC);
        
        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }
}