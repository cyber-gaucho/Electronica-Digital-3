#include 	"lpc17xx_i2c.h"
#include 	"lpc17xx_pinsel.h"
#include 	"lpc17xx_gpio.h"
#include    "lpc17xx_timer.h"
#include    "lpc17xx_adc.h"
#include    "LiquidCrystal_I2C_LPC.h"
#include    <stdlib.h>

typedef unsigned int       uint32_t;
// --- Display ---
#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0
#define     LCD_WIDTH       20
#define     LCD_HEIGHT      4

#define 	LED_RED_PIN     22  // 0.22
#define     LED_BLUE_PIN    26  // 3.26
#define     LED_GREEN_PIN   25  // 3.25

// -------- MENU / OPCIONES ----------
const char headerID[] = "0123456789ABCDE";

const char* itemNames[4] = {
  "Sexo  ",
  "Color ",
  "Categ ",
  "Peso  "
};
const uint8_t xOffSet = 6;  // Ancho de la categoria mas larga

const char* options0[] = {"Macho", "Hembra", "-"};
const char* options1[] = {"Negro", "Colorado", "Careta", "Pampa"};
const char* options2[] = {"Ternero", "Vaquillona", "Novillo", "Toro", "Vaca", "Vaca Prenada"};

const char** itemOptions[3] = {options0, options1, options2};

/**
 * @brief Contiene la cantidad de alternativas de cada opcion.
 * Divide el tamaño del array por el tamaño de un elemento.
 */
const uint8_t itemOptionsCount[3] = { 
  sizeof(options0) / sizeof(options0[0]),
  sizeof(options1) / sizeof(options1[0]),
  sizeof(options2) / sizeof(options2[0])
};

/**
 * @brief Contiene la selección actual para todas las opciones
 */
uint8_t itemSelection[3] = {0, 0, 0};
uint8_t cursorIndex = 0;  // 0..3 (el 3 es lectura, no configurable)

char menu[4][20];
char oldLines[4][20];

void drawMenu();
void initOldLines(void);
static uint8_t lineChanged(uint8_t line, const char *text);
static void copyToOldLine(uint8_t line, const char *text);
void updateLine(uint8_t line, const char *text);
void updateDisplay(void);
void forceUpdate(void);
uint16_t kilos = 123;
// char kilos_str[3]= {"123"};

void delayUs(uint32_t us);

// --- Conf de Perifericos ---
void cfgI2C0(void);

void cfgTimer(void);

/**
* @brief Configuración del ADC (P0.23)
*/
void cfgADC(void);

void cfgLED(void);

int main(void) {
    SystemInit();
    // SysTick_Config(SystemCoreClock / 1000); // 1 ms tick para debounce

    cfgLED();    
	cfgI2C0();

	lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD

	lcd_begin(LCD_I2C_P, LCD_WIDTH, LCD_HEIGHT);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(0, 0); lcd_print("   ED3 - Grupo 1");
    lcd_setCursor(0, 1); lcd_print("  Garcia Lautaro M ");
    lcd_setCursor(0, 2); lcd_print(" Renaudo G Valentino");
    lcd_setCursor(0, 3); lcd_print("  Registro Ganadero");
    delayUs(500000);

    cfgADC();
    cfgTimer();
    lcd_clear();
    initOldLines();
    while (1) {
        // drawMenu();      
        // updateDisplay();
        forceUpdate();
        delayUs(5000);
    }
    
}

void delayUs(uint32_t delayTime){
	for(uint32_t i=0; i<delayTime; i++){
		__NOP();
	}
}

/*=================================================================================*/
/*====================================== LED ======================================*/
/*=================================================================================*/

void cfgLED(void){
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

void setLED(uint8_t r, uint8_t g, uint8_t b){
    if(r) GPIO_ClearValue(0, 1<<LED_RED_PIN); else GPIO_SetValue(0, 1<<LED_RED_PIN);
    if(g) GPIO_ClearValue(3, 1<<LED_GREEN_PIN); else GPIO_SetValue(3, 1<<LED_GREEN_PIN);
    if(b) GPIO_ClearValue(3, 1<<LED_BLUE_PIN); else GPIO_SetValue(3, 1<<LED_BLUE_PIN);
}

/*=================================================================================*/
/*==================================== DISPLAY ====================================*/
/*=================================================================================*/

/* --- Lógica de menú --- */

void drawMenu(void) {
    char kilos_str[3];
    uitoa(kilos, kilos_str, 10);
    
    uint8_t i, j;
    const char* src;

    for (i = 0; i < 4; i++) {
        // Limpia cada línea con espacios
        for (j = 0; j < 20; j++) menu[i][j] = ' ';
        menu[i][19] = '\0';

        j = 0;
        // Indicador de selección
        menu[i][j++] = (i == cursorIndex) ? '>' : ' ';

        // Escribe nombre del ítem
        src = itemNames[i];
        while (*src && j < 20) menu[i][j++] = *src++;

        if (j < 20) menu[i][j++] = ' ';

        // Escribe valor u opción
        if (i < 3) src = itemOptions[i][itemSelection[i]];
        else src = kilos_str;  // usa el string ya convertido

        while (*src && j < 20) menu[i][j++] = *src++;

        // Si es la línea de peso, agrega " kg"
        if (i == 3 && j < 17) {
            menu[i][j++] = ' ';
            menu[i][j++] = 'k';
            menu[i][j++] = 'g';
        }

        // Rellena hasta el final con espacios
        while (j < 19) menu[i][j++] = ' ';
        menu[i][19] = '\0';
    }
}

void initOldLines(void) {
    for (uint8_t r = 0; r < 4; r++) {
        for (uint8_t c = 0; c < 19; c++) oldLines[r][c] = ' ';
        oldLines[r][19] = '\0';
    }
}

// Compara text contra oldLines[line] sin usar strcmp
static uint8_t lineChanged(uint8_t line, const char *text) {
    for (uint8_t i = 0; i < 19; i++) {
        char a = oldLines[line][i];
        char b = text[i];
        if (b == '\0') { // resto debe ser espacios
            // si alguno de los restantes en oldLines no es espacio, hay cambio
            for (uint8_t k = i; k < 19; k++) if (oldLines[line][k] != ' ') return 1;
            return 0; // iguales
        }
        if (a != b) return 1;
    }
    return 0;
}

// Copia text en oldLines[line], rellenando con espacios y colocando '\0'
static void copyToOldLine(uint8_t line, const char *text) {
    uint8_t i = 0;
    for (; i < 19; i++) {
        if (text[i] == '\0') break;
        oldLines[line][i] = text[i];
    }
    // rellena con espacios hasta 19 chars
    for (; i < 19; i++) oldLines[line][i] = ' ';
    oldLines[line][19] = '\0';
}

// Actualiza una línea concreta en el LCD sólo si cambió
void updateLine(uint8_t line, const char *text) {
    if (!lineChanged(line, text)) return;  // nada que hacer

    // actualizamos el buffer viejo
    copyToOldLine(line, text);

    // escribir directamente en la fila: moved cursor y print de 19 caracteres
    lcd_clearRow(line);
    lcd_setCursor(0, line);         // asegurate que la función coloca cursor correctamente
    lcd_print(oldLines[line]);      // tu lcd_print debería aceptar '\0' terminated string
}

// Actualiza las 4 líneas (llamar tras actualizar menu[][] con drawMenu)
void updateDisplay(void) {
    updateLine(0, menu[0]);
    updateLine(1, menu[1]);
    updateLine(2, menu[2]);
    updateLine(3, menu[3]);
}

/**
 * @brief Inicializa el periférico I2C0 a 100kHz.
 */
void cfgI2C0(){
	// Configurar pines P0.27 (SDA0) y P0.28 (SCL0) función 1
    LPC_PINCON->PINSEL1 &= ~((3 << 22) | (3 << 24));
    LPC_PINCON->PINSEL1 |=  ((1 << 22) | (1 << 24));

    // Modo estándar (100–400 kHz)
    LPC_PINCON->I2CPADCFG = 0x00;

    // Inicializar periférico
	I2C_Init(LCD_I2C_P, 100000);

	/* Enable Slave I2C operation */
	I2C_Cmd(LCD_I2C_P, ENABLE);
}

/*=================================================================================*/
/*====================================== ADC ======================================*/
/*=================================================================================*/

/*********************************************************************//**
 * @brief        Configura el ADC en el pin P0.23 para modo normal y habilita
 *               la interrupción en el canal 0.
 * @details      Inicializa el pin como entrada analógica, configura el ADC
 *               a 200 kHz, desactiva burst, selecciona disparo por flanco
 *               descendente, habilita el canal 0 y su interrupción, y activa
 *               la interrupción en el NVIC. Finalmente, inicia el ADC en modo MAT01.
 * @param[in]    Ninguno
 * @return       None
 **********************************************************************/
void cfgADC(){
	PINSEL_CFG_Type pinADC = {0};
	pinADC.Portnum = 0;
	pinADC.Pinnum = 23;
	pinADC.Funcnum = 1;
	pinADC.Pinmode = PINSEL_PINMODE_TRISTATE;
	pinADC.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&pinADC);

	ADC_Init(LPC_ADC, 200000);                           // ADC a 200kHz
	ADC_BurstCmd(LPC_ADC, DISABLE);
	ADC_EdgeStartConfig(LPC_ADC, ADC_START_ON_FALLING);
	ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL_0, ENABLE);     // Habilitar CH 0
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN0, ENABLE);       // Habilitar INT para canal 0
	NVIC_EnableIRQ(ADC_IRQn);                           // Habilitar INT en NVIC
    ADC_StartCmd(LPC_ADC, ADC_START_ON_MAT01);
}

void cfgTimer(){
	TIM_TIMERCFG_Type cfgTimerMode;
	TIM_MATCHCFG_Type cfgTimerMatch;

	cfgTimerMode.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerMode.PrescaleValue = 1000;

	cfgTimerMatch.MatchChannel = 1;
	cfgTimerMatch.MatchValue = 100 - 1;
	cfgTimerMatch.IntOnMatch = DISABLE;
	cfgTimerMatch.ResetOnMatch = ENABLE;
	cfgTimerMatch.StopOnMatch = DISABLE;
	cfgTimerMatch.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;

	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimerMode);
	TIM_ConfigMatch(LPC_TIM0, &cfgTimerMatch);
	TIM_Cmd(LPC_TIM0, ENABLE);
}

/**
* @brief Handler de la interrupción del ADC
*/
void ADC_IRQHandler(void){
	if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL_0, ADC_DATA_DONE)){
		uint16_t adcValue = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL_0);
        if (adcValue == 4095) setLED(1,0,0);
        else setLED(0,0,0);
        kilos = (adcValue * 999) / 4095;  // Mapeo entre 0 y 999
        
		// ADC_ClearIntPending(LPC_ADC, ADC_ADINTEN0); // Clear the ADC interrupt flag
	}
}


void forceUpdate(void) {
    char kilos_str[3];
    uitoa(kilos, kilos_str, 10);
    
    uint8_t i, j;
    const char* src;

    for (i = 0; i < 4; i++) {
        // Limpia cada línea con espacios
        for (j = 0; j < 20; j++) menu[i][j] = ' ';
        menu[i][19] = '\0';

        j = 0;
        // Indicador de selección
        menu[i][j++] = (i == cursorIndex) ? '>' : ' ';

        // Escribe nombre del ítem
        src = itemNames[i];
        while (*src && j < 20) menu[i][j++] = *src++;

        if (j < 20) menu[i][j++] = ' ';

        // Escribe valor u opción
        if (i < 3) src = itemOptions[i][itemSelection[i]];
        else src = kilos_str;  // usa el string ya convertido

        while (*src && j < 20) menu[i][j++] = *src++;

        // Si es la línea de peso, agrega " kg"
        if (i == 3 && j < 17) {
            menu[i][j++] = ' ';
            menu[i][j++] = 'k';
            menu[i][j++] = 'g';
        }

        // Rellena hasta el final con espacios
        while (j < 19) menu[i][j++] = ' ';
        menu[i][19] = '\0';

        lcd_clearRow(i);
        lcd_setCursor(0, i);         // asegurate que la función coloca cursor correctamente
        lcd_print(menu[i]);
    }
}