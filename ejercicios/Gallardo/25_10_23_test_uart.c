#include "lpc17xx_uart.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

char display[] = {"Hola LPC\n"};
volatile uint8_t index;

int main(){
    index = 0;

    configPCB();
    configTimer0();
    configUART();
    while (1){};

    return 0;
}

void configPCB(){
    PINSEL_CFG_Type PinCfg;

    PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);
}

void configUART(){
    UART_CFG_Type UARTConfigStruct;
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    /* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(LPC_UART0, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Enable DMA mode in UART
	UARTFIFOConfigStruct.FIFO_DMAMode = ENABLE;

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);

	// Enable UART Transmit
	UART_TxCmd(LPC_UART0, ENABLE);
}

void configTimer0(){
    TIM_TIMERCFG_Type cfgTimer;
    TIM_MATCHCFG_Type cfgMatch;

    cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
    cfgTimer.PrescaleValue = 1000; //1ms

    cfgMatch.MatchChannel = 0; 
    cfgMatch.IntOnMatch = ENABLE;
    cfgMatch.StopOnMatch = DISABLE;
    cfgMatch.ResetOnMatch = ENABLE;
    cfgMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    cfgMatch.MatchValue = 1000; //Match cada 1s

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer);
    TIM_ConfigMatch(LPC_TIM0, &cfgMatch);
    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

Timer0_IRQHandler(){
    if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
        UART_Send(LPC_UART0, &display, 10, NONE_BLOCKING);
        
        // UART_SendByte(LPC_UART0, (uint8_t)display[index]);
        // index++;
        // index = index & 0x1F; // Procuramos index dentro de rango

    }
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}



