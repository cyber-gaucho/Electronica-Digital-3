/*
===============================================================================
 Name        : DMA.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_adc.h"
#define DMA_SWAP_SIZE 10
#define TABLE_LEN 1034



void confDMA(void);
void confADC(void); //Prototipo de la función de conf. de interrupciones externas


GPDMA_Channel_CFG_Type GPDMACfg;

uint32_t table[TABLE_LEN];
uint32_t random[2];

uint32_t aux_buffer[DMA_SWAP_SIZE];

GPDMA_LLI_Type DMA_LLI_Struct1;
GPDMA_LLI_Type DMA_LLI_Struct2;
GPDMA_LLI_Type DMA_LLI_Struct3;

uint32_t bitcount;


int main(void)
{
	uint32_t i;
	random[0]=545;
	random[1]=433;
	bitcount=0;

	//Prepare  table
	for(i=0;i<TABLE_LEN;i++)
	{
		table[i] = i;
	}
	confDMA();
	confADC();

	while (1);
	return 0;
}





void confDMA(void){

	//Prepare DMA link list item structure
	DMA_LLI_Struct1.SrcAddr= (uint32_t)table+random[0];
	DMA_LLI_Struct1.DstAddr= (uint32_t)aux_buffer;
	DMA_LLI_Struct1.NextLLI= (uint32_t)&DMA_LLI_Struct2; //0;
	DMA_LLI_Struct1.Control= DMA_SWAP_SIZE
			| (2<<18) //source width 32 bit
			| (2<<21) //dest. width 32 bit
			| (1<<26) //source increment
			| (1<<27) //dest increment
			;
	DMA_LLI_Struct2.SrcAddr= (uint32_t)table+random[1];
	DMA_LLI_Struct2.DstAddr= (uint32_t)table+random[0];
	DMA_LLI_Struct2.NextLLI= (uint32_t)&DMA_LLI_Struct3; //0;
	DMA_LLI_Struct2.Control= DMA_SWAP_SIZE
			| (2<<18) //source width 32 bit
			| (2<<21) //dest. width 32 bit
			| (1<<26) //source increment
			| (1<<27) //dest increment
			;

	DMA_LLI_Struct3.SrcAddr= (uint32_t)aux_buffer;
	DMA_LLI_Struct3.DstAddr= (uint32_t)table+random[1];
	DMA_LLI_Struct3.NextLLI= 0; //0;
	DMA_LLI_Struct3.Control= DMA_SWAP_SIZE
			| (2<<18) //source width 32 bit
			| (2<<21) //dest. width 32 bit
			| (1<<26) //source increment
			| (1<<27) //dest increment
			;


	/* GPDMA block section -------------------------------------------- */
	/* Initialize GPDMA controller */
	GPDMA_Init();
	// Setup GPDMA channel --------------------------------
	// channel 7
	GPDMACfg.ChannelNum = 7;
	// Source memory
	GPDMACfg.SrcMemAddr = DMA_LLI_Struct1.SrcAddr;
	// Destination memory -
	GPDMACfg.DstMemAddr = DMA_LLI_Struct1.DstAddr;
	// Transfer size
	GPDMACfg.TransferSize = DMA_SWAP_SIZE;
	// Transfer width - wrd
	GPDMACfg.TransferWidth = GPDMA_WIDTH_WORD;
	// Transfer type
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2M;
	// Source connection - unused
	GPDMACfg.SrcConn = 0;
	// Destination connection - unused
	GPDMACfg.DstConn = 0;
	// Linker List Item - unused
	GPDMACfg.DMALLI = (uint32_t)&DMA_LLI_Struct2;
	// Setup channel with given parameter
	GPDMA_Setup(&GPDMACfg);
	return;
}


void confADC(void){
	ADC_Init(LPC_ADC, 2000);
	LPC_SC->PCONP |= (1 << 12);
	LPC_ADC->ADCR |= (1 << 21);
	LPC_SC->PCLKSEL0 |= (3<<24); //CCLK/8
	LPC_ADC->ADCR &=~(255 << 8); //[15:8] CLKDIV
	LPC_ADC->ADCR |= (1 << 16);
	LPC_PINCON->PINMODE1 |= (1<<15); //neither pull-up nor pull-down.
	LPC_PINCON->PINSEL1 |= (1<<14); //0.23 AD0.0
	LPC_ADC->ADINTEN = 1;
	NVIC_SetPriority(ADC_IRQn, 3);
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void ADC_IRQHandler(void) {
	static uint16_t ADC0Value = 0;
	ADC0Value = ((LPC_ADC->ADDR0)>>4) & 0x001; //Variable auxiliar para observar el valor del registro de captura
	if(bitcount< 10){
		if(bitcount==0){
			random[0]=0;
		}
		random[0]=(random[0]<<1)|ADC0Value;
		bitcount++;
	}else{
		if(bitcount==10){
			random[1]=0;
		}
		random[1]=(random[1]<<1)|ADC0Value;
		bitcount++;
		if(bitcount==20){
			bitcount=0;
			random[1]=random[1]%1024;
			random[0]=random[0]%1024;
			NVIC_DisableIRQ(DMA_IRQn);
			confDMA();
			// Enable GPDMA channel 5
			GPDMA_ChannelCmd(7, ENABLE);
		}
	}
	return;
}
