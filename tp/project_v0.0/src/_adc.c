#include "_adc.h"

void adc_init(void){
    PINSEL_CFG_Type pinADC = {0};
	pinADC.Portnum = 0;
	pinADC.Pinnum = 23;
	pinADC.Funcnum = 1;
	pinADC.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinADC.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinADC);

	ADC_Init(LPC_ADC, ADC_RATE);                           // ADC a 200kHz
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);     // Habilitar CH 0
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);       // Habilitar INT para canal 0
	// NVIC_EnableIRQ(ADC_IRQn);                           // Habilitar INT en NVIC
    // ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);

    TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerMode.PrescaleValue = 1000;

	cfgTimerMatch.MatchChannel = 1;
	cfgTimerMatch.MatchValue = ADC_TIMER_MATCH_VALUE - 1;
	cfgTimerMatch.IntOnMatch = DISABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
	adc_comand(ENABLE);
}


void adc_comand(FunctionalState NewState){
    if(NewState == ENABLE){
        ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
        TIM_Cmd(LPC_TIM0, ENABLE);
        NVIC_EnableIRQ(ADC_IRQn);
    } else {
        TIM_Cmd(LPC_TIM0, DISABLE);
        ADC_StopCmd(LPC_ADC, ADC_START_ON_MAT01);
        TIM_ResetCounter(LPC_TIM0);
        TIM_Cmd(LPC_TIM0, DISABLE);
        NVIC_DisableIRQ(ADC_IRQn);
    }
}

void ADC_IRQHandler(void){
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){
		uint16_t adcValue = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        if (adcValue == 4095) setLED(1,0,0);
        else setLED(0,0,0);
        kilos = (adcValue * 999) / 4095;  // Mapeo entre 0 y 999
        
		// ADC_ClearIntPending(LPC_ADC, ADC_ADINTEN0); // Clear the ADC interrupt flag
	}
}