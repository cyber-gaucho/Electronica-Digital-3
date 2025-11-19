#include "utils.h"

#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"

volatile uint8_t delay_flag = 0;    // Flag para manejo de retardo por timer

/* Private Functions ---------------------------------------------------------- */
/**
 * @brief Configuración detallada de TIM2 para que interrumpa en "ms" ms.
 *
 * @param ms Tiempo en ms al cual se generará la interrupción (match)
 */
static void TIM2_init(uint32_t ms){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;	// Unidad: us
	cfgTimerMode.PrescaleValue = 1000;					// 1000 us = 1 ms

	cfgTimerMatch.MatchChannel = 0;					// Usar MR0
	cfgTimerMatch.IntOnMatch = ENABLE;				// Habilita interrupción
	cfgTimerMatch.StopOnMatch = ENABLE;				// Detener
	cfgTimerMatch.ResetOnMatch = ENABLE;		    // Reiniciar
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    cfgTimerMatch.MatchValue = (ms == 0) ? 1 : ms;	        // evita underflow

	TIM_Init(LPC_TIM2, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM2, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM2, ENABLE);							// Arranca el timer

	NVIC_EnableIRQ(TIMER2_IRQn);						// Habilita IRQ en NVIC
}

/* End of Private Functions ---------------------------------------------------- */

/* Public Functions ----------------------------------------------------------- */

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
	TIM_DeInit(LPC_TIM2);
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