#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"

/**********************************************************************
 * @file    test_DAC_DMA.c
 * @brief   Generación de tonos mediante DMA y DAC en LPC17xx.
 * 
 * Este archivo configura el DAC usando DMA para reproducir una onda 
 * senoidal almacenada en una tabla, controlando la duración y frecuencia 
 * mediante timers e interrupciones. Incluye rutinas de inicialización 
 * y un loop principal de prueba.
 * 
 * @author  
 * @date    
 **********************************************************************/

#define SAMPLES_AMOUNT      256     // Cantidad de muestras de la tabla de seno
#define SINE_FREQ_IN_HZ     440     // Frecuencia target en Hz (tono La4)
#define PCLK_DAC_IN_MHZ     25      // Frecuencia de clock periférico del DAC (MHz)

/* LED Pin Configuration */
#define LED_RED_PIN     22  // P0.22
#define LED_BLUE_PIN    26  // P3.26
#define LED_GREEN_PIN   25  // P3.25

volatile uint8_t delay_flag = 0;    // Flag para manejo de retardo por timer

/**
 * @brief Tabla con muestras del seno desplazadas para el registro DACR
 * 
 * Los valores están desplazados para encajar en el registro de 16 bits del DAC,
 * generando así una onda senoidal adecuada para salida analógica.
 */
uint32_t sine_bank[SAMPLES_AMOUNT] = {
	32768, 33536, 34368, 35136, 35968, 36736, 37568, 38336, 39104, 39936, 40704, 41472,
	42240, 43008, 43776, 44544, 45248, 46016, 46720, 47424, 48192, 48896, 49536, 50240,
	50944, 51584, 52224, 52864, 53504, 54144, 54720, 55296, 55872, 56448, 56960, 57536,
	58048, 58560, 59008, 59520, 59968, 60416, 60800, 61248, 61632, 61952, 62336, 62656,
	62976, 63296, 63552, 63808, 64064, 64256, 64512, 64704, 64832, 64960, 65088, 65216,
	65344, 65408, 65408, 65472, 65472, 65472, 65408, 65408, 65344, 65216, 65088, 64960,
	64832, 64704, 64512, 64256, 64064, 63808, 63552, 63296, 62976, 62656, 62336, 61952,
	61632, 61248, 60800, 60416, 59968, 59520, 59008, 58560, 58048, 57536, 56960, 56448,
	55872, 55296, 54720, 54144, 53504, 52864, 52224, 51584, 50944, 50240, 49536, 48896,
	48192, 47424, 46720, 46016, 45248, 44544, 43776, 43008, 42240, 41472, 40704, 39936,
	39104, 38336, 37568, 36736, 35968, 35136, 34368, 33536, 32768, 31936, 31104, 30336,
	29504, 28736, 27904, 27136, 26368, 25536, 24768, 24000, 23232, 22464, 21696, 20928,
	20224, 19456, 18752, 18048, 17280, 16576, 15936, 15232, 14528, 13888, 13248, 12608,
	11968, 11328, 10752, 10176, 9600, 9024, 8512, 7936, 7424, 6912, 6464, 5952,
	5504, 5056, 4672, 4224, 3840, 3520, 3136, 2816, 2496, 2176, 1920, 1664,
	1408, 1216, 960, 768, 640, 512, 384, 256, 128, 64, 64, 0,
	0, 0, 64, 64, 128, 256, 384, 512, 640, 768, 960, 1216,
	1408, 1664, 1920, 2176, 2496, 2816, 3136, 3520, 3840, 4224, 4672, 5056,
	5504, 5952, 6464, 6912, 7424, 7936, 8512, 9024, 9600, 10176, 10752, 11328,
	11968, 12608, 13248, 13888, 14528, 15232, 15936, 16576, 17280, 18048, 18752, 19456,
	20224, 20928, 21696, 22464, 23232, 24000, 24768, 25536, 26368, 27136, 27904, 28736,
	29504, 30336, 31104, 31936
};

/**
 * @brief Inicializa el DAC y su pin de salida.
 * 
 * Configura P0.26 como salida de DAC, activa modo de contador y DMA
 * en el convertidor digital analógico.
 */
void dac_init(void);            

/**
 * @brief Configura canal de DMA para el DAC usando una lista ligada.
 * 
 * Prepara una transferencia circular de la tabla de muestras al registro del DAC,
 * utilizando el canal 0 del DMA.
 */
void configDMA_DAC_Channel(void);

/**
 * @brief Inicializa el timer 2 (TIM2) para generar un retardo específico en ms.
 * 
 * @param ms Retardo en milisegundos.
 */
void TIM2_init(uint32_t ms);

/**
 * @brief Retardo bloqueante utilizando TIM2 (milisegundos).
 *
 * Espera activa utilizando un flag modificado por la ISR del TIMER2.
 *
 * @param ms Tiempo a esperar en milisegundos.
 */
void delayTIM2(uint32_t ms);

/**
 * @brief Genera un tono a una frecuencia dada por un cierto tiempo.
 * 
 * @param frec_Hz Frecuencia en Hz de la onda senoidal.
 * @param ms Duración en milisegundos del tono generado.
 */
void generateTone(uint16_t frec_Hz, uint32_t ms);

/**
 * @brief Configuración de los pines de los LEDs
 * @details Inicializa los pines de los LEDs como salidas
 * @param[in] Ninguno
 * @return None
 */
 void LED_init(void);

 /**
  * @brief Enciende o apaga los LEDs
  * @param[in] r: Valor del LED rojo
  * @param[in] g: Valor del LED verde
  * @param[in] b: Valor del LED azul
  * @note Enciende el LED con VALOR 1 y apaga con VALOR 0
  * @return None
  */
 void LED_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Función principal.
 * 
 * Inicializa el sistema y genera un tono de 440 Hz por 3 segundos,
 * luego espera 10 segundos y repite el proceso.
 */
int main() {
	SystemInit();

	dac_init();	// Inicializar DAC y configurar pin

	while(1){
		generateTone(SINE_FREQ_IN_HZ, 3000);	// 440 Hz durante 3 segundos
		delayTIM2(10000);						// Espera bloqueante de 10 segundos
	}
}

/**
 * @brief Inicializa el DAC y configuración de pin de salida P0.26.
 */
void dac_init(){
	// Configuración de P0.26 como salida analógica del DAC
	PINSEL_CFG_Type pinCfg;
	pinCfg.Funcnum = 2;		// Función analógica (DAC)
	pinCfg.OpenDrain = 0;
	pinCfg.Pinmode = 0;
	pinCfg.Portnum = 0;
	pinCfg.Pinnum = 26;
	PINSEL_ConfigPin(&pinCfg);

	DAC_CONVERTER_CFG_Type dacCfg;
	dacCfg.CNT_ENA = SET;	// Habilitar modo de contador
	dacCfg.DMA_ENA = SET;	// Habilitar DMA
	DAC_Init(LPC_DAC);

	// Configuración del control del convertidor DA
	DAC_ConfigDAConverterControl(LPC_DAC, &dacCfg);
}

/**
 * @brief Inicializa y configura el canal de DMA 0 para transferencias M2P a DAC.
 */
void configDMA_DAC_Channel(){
	//------ Configuración de la Linked List del DMA ------
	// - Source width: 32 bits
	// - Dest width: 32 bits
	// - Source address se incrementa, destino (DAC) fijo
	GPDMA_LLI_Type LLI1;
	LLI1.SrcAddr = (uint32_t) sine_bank;
	LLI1.DstAddr = (uint32_t) &LPC_DAC->DACR;
	LLI1.NextLLI = (uint32_t) &LLI1;	// Circular (auto-loop)
	LLI1.Control = SAMPLES_AMOUNT | (1<<19) | (1<<22) | (1<<26);

	GPDMA_Init();

	// Configuración y habilitación del Canal 0 de DMA
	GPDMA_Channel_CFG_Type GPDMACfg;
	GPDMACfg.ChannelNum = 0;
	GPDMACfg.SrcMemAddr = (uint32_t)sine_bank;
	GPDMACfg.DstMemAddr = 0;	// Cuando es M2P, no se usa
	GPDMACfg.TransferSize = SAMPLES_AMOUNT;
	GPDMACfg.TransferWidth = 0;	// Uso por defecto (lo define control del LLI)
	GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
	GPDMACfg.SrcConn = 0;
	GPDMACfg.DstConn = GPDMA_CONN_DAC;
	GPDMACfg.DMALLI = (uint32_t)&LLI1;
	GPDMA_Setup(&GPDMACfg);
}

/**
 * @brief Configura el DMA y DAC para generar un tono de frecuencia dada y duración ms.
 * 
 * @param frec_Hz Frecuencia del tono (Hz)
 * @param ms Tiempo en milisegundos a mantener el tono
 */
void generateTone(uint16_t frec_Hz,uint32_t ms){
	configDMA_DAC_Channel();

	// Ajusta el parámetro de timeout del DAC para frecuencia y cantidad de muestras
	uint32_t tmp;
	tmp = (PCLK_DAC_IN_MHZ * 1000000)/(frec_Hz * SAMPLES_AMOUNT);
	DAC_SetDMATimeOut(LPC_DAC, tmp);

	// Habilita transferencia por DMA
	GPDMA_ChannelCmd(0, ENABLE);
	LED_set(0,0,1);

	delayTIM2(ms);

	LED_set(0,0,0);

	// Detiene transferencia por DMA
	GPDMA_ChannelCmd(0, DISABLE);
}

/**
 * @brief Retardo bloqueante basado en timer 2.
 *
 * Ejecuta delay_flag en espera activa; la bandera se limpia por interrupción del timer.
 * 
 * @param ms Milisegundos a esperar
 */
void delayTIM2(uint32_t ms){
	delay_flag = 1;
	TIM2_init(ms);	// Configura y arranca el timer

	while(delay_flag);	// Espera hasta que la ISR limpie la bandera

	// Limpieza al concluir delay
	TIM_Cmd(LPC_TIM2, DISABLE);
	TIM_ResetCounter(LPC_TIM2);
	TIM_DeInit(LPC_TIM2);
}

/**
 * @brief Configuración detallada de TIM2 para que interrumpa en "ms" ms.
 *
 * @param ms Tiempo en ms al cual se generará la interrupción (match)
 */
void TIM2_init(uint32_t ms){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;	// Unidad: us
	cfgTimerMode.PrescaleValue = 1000;					// 1000 us = 1 ms

	cfgTimerMatch.MatchChannel = 0;					// Usar canal MR0
	cfgTimerMatch.IntOnMatch = ENABLE;					// Habilita interrupción al match
	cfgTimerMatch.StopOnMatch = DISABLE;				// No detener el timer al match
	cfgTimerMatch.ResetOnMatch = DISABLE;				// No reiniciar el timer al match
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; // Ninguna salida extra
	cfgTimerMatch.MatchValue = ms - 1;					// Valor de comparación: ms ciclos

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM2, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM2, ENABLE);							// Arranca el timer

	NVIC_EnableIRQ(TIMER2_IRQn);						// Habilita IRQ en NVIC
}

/**
 * @brief Rutina de Interrupción para TIMER2.
 * 
 * Limpia flag de delay cuando ocurre el match en MR0, permitiendo continuar el flujo bloqueante.
 */
void TIMER2_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM2, TIM_MR0_INT) == SET){
		delay_flag = 0;							// Libera espere activa en delayTIM2
		TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);// Limpia el flag de interrupción
	}
}

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