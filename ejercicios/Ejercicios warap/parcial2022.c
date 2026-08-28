/***EJERCICIO Nº 1: (30 %)**
Programar el microcontrolador LPC1769 para que mediante su ADC digitalice  dos señales analógicas cuyos anchos 
de bandas son de 10 Khz cada una. Los canales utilizados deben ser el 2 y el 4 y los datos deben ser guardados 
en dos regiones de memorias distintas que permitan contar con los últimos 20 datos de cada canal. 
Suponer una frecuencia de core cclk de 100 Mhz. El código debe estar debidamente comentado.*/
#include "LPC17xx.h" 
#include "lpc17xx_adc.h" 
#include "lpc17xx_gpdma.h" 
#include "lpc17xx_pinsel.h" 

void cfgADC(void); 
void cfgDMA(void); 

#define MUESTRAS 20 
#define ADC_CH2_ADDR ((uint32_t)&(LPC_ADC->ADDR2))
#define ADC_CH4_ADDR ((uint32_t)&(LPC_ADC ->ADDR4))

uint32_t buffer_2[MUESTRAS]; // Para el canal 2 del ADC
uint32_t buffer_4[MUESTRAS];  // Para el canal 4 del ADC 


int main(void){

    while(1){}
}

void cfgADC(void){
     
    /* Configurar 
    canal 2: P0.25
    canal 4: P1.30*/
    ADC_Init(30000); // fs = 30kHZ (30 muestras por segundo)
    
    PINSEL_CFG_Type pincfg; 
  
    pincfg.portNum = PINSEL_PORT_0;
    pincfg.pinNum  = PINSEL_PIN_25;
    pincfg.funcNum = PINSEL_FUNC_1;
    pincfg.pinMode = PINSEL_TRISTATE; 
    pincfg.openDrain = PINSEL_OD_NORMAL; 
    PINSEL_ConfigPin(&pincfg);

    pincfg.portNum = PINSEL_PORT_1;
    pincfg.pinNum  = PINSEL_PIN_30;
    pincfg.funcNum = PINSEL_FUNC_3;
    pincfg.pinMode = PINSEL_TRISTATE; 
    pincfg.openDrain = PINSEL_OD_NORMAL; 
    PINSEL_ConfigPin(&pincfg);

    ADC_ChannelCmd(ADC_CHANNEL_2, ENABLE); 
    ADC_ChannelCmd(ADC_CHANNEL_4, ENABLE); 

    ADC_BurstCmd(ENABLE); 
    
}

void cfgDMA(void){
    /*Transferencia P2M*/
    GPDMA_Init();
    GPDMA_Channel_CFG_Type cfgDMA_2; 
    GPDMA_Channel_CFG_Type cfgDMA_4; 
    GPDMA_LLI_Type LLI0; 
    GPDMA_LLI_Type LLI1; 

    cfgDMA_2.ChannelNum = 0; 
    cfgDMA_2.TransferSize = MUESTRAS; 
    cfgDMA_2.TransferType = GPDMA_TRANSFERTYPE_P2M; 
    cfgDMA_2.TransferWidth = GPDMA_WIDTH_WORD; 
    cfgDMA_2.SrcMemAddr = 0; 
    cfgDMA_2.DstMemAddr = buffer_2; 
    cfgDMA_2.SrcConn = GPDMA_CONN_ADC; 
    cfgDMA_2.DstConn = 0; 
    
    GPDMA_Setup(&cfgDMA_2);
    GPDMA_ChannelCmd(0, ENABLE); 

    /*LLI PARA EL CANAL 2*/
    LLI0.SrcAddr = ADC_CH2_ADDR; 
    LLI0.DstAddr = (uint32_t)buffer_2; 
    LLI0.NextLLI = (uint32_t)&LLI0; // Circular
    LLI0.Control = (MUESTRAS)| 
                   (GPDMA_WIDTH_WORD << 18)| // Transfer width (src)
                   (GPDMA_WIDTH_WORD<<21)| // Transfer width (dst)
                   (0<<26) | // SI -> NO incremento fuente
                   (1<<27) | // DI -> SI incremento destino 
                   (1<<31); // Terminal Count Interrupt Enable
    
    cfgDMA_2.DMALLI = (uint32_t)&LLI0;

    cfgDMA_4.ChannelNum = 1; 
    cfgDMA_4.TransferSize = MUESTRAS; 
    cfgDMA_4.TransferType = GPDMA_TRANSFERTYPE_P2M;
    cfgDMA_4.TransferWidth = GPDMA_WIDTH_WORD; 
    cfgDMA_4.SrcMemAddr = 0; 
    cfgDMA_4.DstMemAddr = buffer_4; 
    cfgDMA_4.SrcConn = GPDMA_CONN_ADC; 
    cfgDMA_4.DstConn = 0; 

    GPDMA_Setup (&cfgDMA_4);
    GPDMA_ChannelCmd (1, ENABLE); 

    /*LLI PARA EL CANAL 4*/
    LLI1.SrcAddr = ADC_CH4_ADDR; 
    LLI1.DstAddr = (uint32_t)(buffer_4);
    LLI1.NextLLI = (uint32_t)&LLI1; // Circular 
    LLI1.Control = LLI0.Control; 

    cfgDMA_4.DMALLI = (uint32_t)&LLI1;
   
}

void DMA_IRQHandler(void){

    volatile uint8_t flag_2 = 0; 
    volatile uint8_t flag_4 = 0; 

    /*Canal 0: ADC del canal 2*/
    if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)) /*El canal termino de transferir 
    la cantidad total de datos que se le pidio*/ {
        GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0); // Limpia el flag
     flag_2 = 1; 
    }

    /*Canal 1: ADC del canal 4*/
    if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1)){
        GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC,1); // Limpia el flag 
        flag_4 = 1;
    }

    /* En caso de error */
    if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0) 
    || (GPDMA_IntGetStatus (GPDMA_STAT_INTERR,1))) {
        GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 0);
        GPDMA_ClearIntPending(GPDMA_STATCLR_INTERR, 1);
    }

}
