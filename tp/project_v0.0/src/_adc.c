#include "_adc.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"
void adc_init(void){
    PINSEL_CFG_Type pinADC = {0};
	pinADC.Portnum = 0;
	pinADC.Pinnum = 23;
	pinADC.Funcnum = 1;
	pinADC.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinADC.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinADC);

	ADC_Init(LPC_ADC, ADC_RATE);                           // ADC a 200kHz
	ADC_BurstCmd(LPC_ADC, ENABLE);                         // Enable burst mode
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);
}