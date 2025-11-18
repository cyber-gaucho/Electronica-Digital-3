/**
 * @file tp.c
 * @brief
 * 
 */

#include 	"lpc17xx_i2c.h"
#include 	"lpc17xx_pinsel.h"
#include 	"lpc17xx_gpio.h"
#include    "lpc17xx_timer.h"
#include    "lpc17xx_adc.h"
#include    "LiquidCrystal_I2C_LPC.h"
#include    <stdlib.h>
#include    <string.h>

// --- Display ---
#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0
#define     LCD_WIDTH       20
#define     LCD_HEIGHT      4

#define 	LED_RED_PIN     22  // 0.22
#define     LED_BLUE_PIN    26  // 3.26
#define     LED_GREEN_PIN   25  // 3.25

// --- Botones ---
#define     BTN_PORT        2
// #define BTN_UP_PIN      0
// #define BTN_DOWN_PIN    1
// #define BTN_RIGHT_PIN   2
// #define BTN_LEFT_PIN    3
// #define BTN_SAVE_PIN    4
// #define BTN_SEND_PIN    5
/**
 * @enum BTN_OPT
 * @brief Enumeración de pines asociados a botones de control.
 *
 * Esta enumeración define los identificadores de los pines utilizados para los botones.
 * El valor BTN_ID_PIN debe permanecer como el último elemento ya que se usa en un for loop.
 *
 * Nota: El pin P2.9 no está disponible en la placa.
 */
enum {
    BTN_UP_PIN = 0,
    BTN_DOWN_PIN, 
    BTN_RIGHT_PIN,
    BTN_LEFT_PIN,
    BTN_SAVE_PIN,
    BTN_SEND_PIN,
    BTN_ID_PIN      // Dejar ID ultimo en este enum
} BTN_OPT;

volatile uint8_t action = 0;
volatile uint8_t firstID = 0;

// --- Pin analógico ---
#define LOAD_CELL_PORT  0
#define LOAD_CELL_PIN   23  // Entrada analógica (potenciómetro)

// --- PARÁMETROS ---
#define SERIAL_BAUD     9600
#define DEBOUNCE_MS     25
#define BLINK_INTERV_MS 400
#define UI_REFRESH_MS   100


// --- ADC ---
#define BUFFER_SIZE 32
volatile uint16_t adcBuffer[BUFFER_SIZE];
volatile uint32_t adcBufferIndex = 0;
volatile bool new_sample_flag = false;
uint16_t kilos = 0;
char kilos_str[3];
// -------- MENU / OPCIONES ----------
const char headerID[] = "0123456789ABCDE";

const char* itemNames[4] = {
  "Sexo  ",
  "Color ",
  "Categ ",
  "Peso  "
};
const uint8_t xOffSet = 6;  // Ancho de la categoria mas larga

const char* options0[] = {"-", "Macho", "Hembra"};
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

// --- Estructura de datos ---
typedef struct {
    uint32_t id;       // ID leído del RFID
    uint8_t tipo;      // índice de tipo seleccionado
    uint8_t estado;    // índice de estado
    uint8_t categoria; // índice de categoría
    int pesoKg;        // peso cargado
} Registro;

// ---- Nodo del árbol binario ----
typedef struct Nodo {
    Registro data;
    struct Nodo* izq;
    struct Nodo* der;
} Nodo;

Nodo* crearNodo(Registro r);
Nodo* insertarNodo(Nodo* raiz, Registro r);
Nodo* buscarNodo(Nodo* raiz, uint32_t id);
void recorrerInOrden(Nodo* raiz);

void drawMenu();
void initOldLines(void);
static uint8_t lineChanged(uint8_t line, const char *text);
static void copyToOldLine(uint8_t line, const char *text);
void updateLine(uint8_t line, const char *text);
void updateDisplay(void);

void handleAction(void);
void setupButtons(void);
void cfgPin(void); // Otros pines como LED
void setLED(uint8_t r, uint8_t g, uint8_t b);

void cfgI2C0(void);
/**
* @brief Configuración del ADC (P0.23)
*/
void cfgADC();

void cfgTimer();

static void delayUs(uint32_t us);

int main(void) {
    SystemInit();
    // SysTick_Config(SystemCoreClock / 1000); // 1 ms tick para debounce
        
	cfgI2C0();

	lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD

	lcd_begin(LCD_I2C_P, LCD_WIDTH, LCD_HEIGHT);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(0, 0); lcd_print("   ED3 - Grupo 1");
    lcd_setCursor(0, 1); lcd_print("  Garcia Lautaro M ");
    lcd_setCursor(0, 2); lcd_print(" Renaudo G Valentino");
    lcd_setCursor(0, 3); lcd_print("  Registro Ganadero");

    cfgADC();
    cfgTimer();
    cfgPin();
    setupButtons();
    while (!firstID);
    while (1) {
        drawMenu();
        updateDisplay();
        handleAction();
        delayUs(5000);
    }
}

static void delayUs(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    for (volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();
    }
}

void cfgPin(void){
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
/*==================================== Botones ====================================*/
/*=================================================================================*/

/**
 * @brief Configura pines e interrupciones para los botones
 */
void setupButtons(void) {
    PINSEL_CFG_Type cfgButtons = {0};
    cfgButtons.Portnum = BTN_PORT;
    cfgButtons.Funcnum = PINSEL_FUNC_0; // GPIO
    cfgButtons.Pinmode = PINSEL_PINMODE_PULLUP;
    cfgButtons.OpenDrain = PINSEL_PINMODE_NORMAL;
    for(uint8_t i = 0; i <= BTN_ID_PIN; i++){
        cfgButtons.Pinnum = i;
        PINSEL_ConfigPin(&cfgButtons);
    }
    LPC_GPIO2->FIODIR &= ~((1 << BTN_UP_PIN) | (1 << BTN_DOWN_PIN) | (1 << BTN_RIGHT_PIN) |
                       (1 << BTN_LEFT_PIN) | (1 << BTN_SAVE_PIN) | (1 << BTN_SEND_PIN) |
                       (1 << BTN_ID_PIN));

    GPIO_ClearInt(2,0xFFFF);  // Limpia cualquier interrupción previa
    GPIO_IntCmd(2,((1 << BTN_UP_PIN) | (1 << BTN_DOWN_PIN) | (1 << BTN_RIGHT_PIN) |
                    (1 << BTN_LEFT_PIN) | (1 << BTN_SAVE_PIN) | (1 << BTN_SEND_PIN) |
                    (1 << BTN_ID_PIN)),1);
    NVIC_EnableIRQ(EINT3_IRQn);
}
/**
 * @brief Handler de las interrupciones por GPIO para los botones
 */
void EINT3_IRQHandler(void) {
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_UP_PIN)) { action = 1; LPC_GPIOINT->IO2IntClr = (1 << BTN_UP_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_DOWN_PIN)) { action = 2; LPC_GPIOINT->IO2IntClr = (1 << BTN_DOWN_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_RIGHT_PIN)) { action = 3; LPC_GPIOINT->IO2IntClr = (1 << BTN_RIGHT_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_LEFT_PIN)) { action = 4; LPC_GPIOINT->IO2IntClr = (1 << BTN_LEFT_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_SAVE_PIN)) { action = 5; LPC_GPIOINT->IO2IntClr = (1 << BTN_SAVE_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_SEND_PIN)) { action = 6; LPC_GPIOINT->IO2IntClr = (1 << BTN_SEND_PIN); }
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_ID_PIN)) { action = 7; firstID = 1; LPC_GPIOINT->IO2IntClr = (1 << BTN_ID_PIN); }
}

/* --- Acciones --- */
void handleAction(void) {
    // static uint32_t lastActionTime = 0;
    if (action == 0) return;

    // if ((SysTick->VAL - lastActionTime) < 150) return;
    // lastActionTime = SysTick->VAL;

    switch (action) {
        case 1: // UP
            if (cursorIndex > 0) cursorIndex--;
            break;
        case 2: // DOWN
            if (cursorIndex < 3) cursorIndex++;
            break;
        case 3: // RIGHT
            if(cursorIndex != 3) {
                itemSelection[cursorIndex]++;
                if (itemSelection[cursorIndex] > itemOptionsCount[cursorIndex]) itemSelection[cursorIndex] = 0;
            }
            break;
        case 4: // LEFT
            if(cursorIndex != 3) {
                if (itemSelection[cursorIndex] == 0) itemSelection[cursorIndex] = itemOptionsCount[cursorIndex] - 1;
                else itemSelection[cursorIndex]--;
            }
            break;
        case 5: // SAVE
            // TODO
            setLED(1, 0, 0);
            break;
        case 6: // SEND
            // TODO
            setLED(0, 0, 1);
            break;
        case 7: // ID
            // TODO
            setLED(0, 1, 0);
            break;
    }
    action = 0;
}

/*=================================================================================*/
/*==================================== DISPLAY ====================================*/
/*=================================================================================*/

/* --- Lógica de menú --- */

void drawMenu(void) {
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
    lcd_setCursor(0, line);         // asegurate que la función colocca cursor correctamente
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

	ADC_Init(LPC_ADC, 20000);                           // ADC a 200kHz
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
        kilos = (adcValue * 999) / 4095;  // Mapeo entre 0 y 999
        uitoa(kilos, kilos_str, 10);
		// ADC_ClearIntPending(LPC_ADC, ADC_ADINTEN0); // Clear the ADC interrupt flag
	}
}

/*=================================================================================*/
/*===================================== UART ======================================*/
/*=================================================================================*/

/**
 * @brief Configura UART0 para transmitir...
 */
void init_uart0(uint32_t baudrate) {
    // 1. Encender periférico (Power Control)
    LPC_SC->PCONP |= (1 << 3); // PCUART0 = 1

    // 2. Configurar pines (Pin Select)
    LPC_PINCON->PINSEL0 |= (1 << 4); // P0.2 como TXD0
    LPC_PINCON->PINSEL0 |= (1 << 6); // P0.3 como RXD0

    // 3. Configurar formato (Line Control Register)
    LPC_UART0->LCR = 0x83; // 8-N-1, DLAB = 1

    // 4. Configurar Baudrate
    uint32_t PCLK_UART = 25000000; // Asumir PCLK = 25MHz
    uint32_t Fdiv = (PCLK_UART) / (16 * baudrate);
    LPC_UART0->DLL = Fdiv & 0xFF;
    LPC_UART0->DLM = (Fdiv >> 8) & 0xFF;

    // 5. Deshabilitar DLAB y habilitar FIFO
    LPC_UART0->LCR = 0x03; // DLAB = 0
    LPC_UART0->FCR = 0x07; // Habilitar y resetear FIFOs
}

/**
 * @brief Envía un string por UART0...
 */
void UART0_EnviarString(char* str) {
    while (*str != '\0') {
        while (!(LPC_UART0->LSR & 0x20)); // Esperar hasta que THR esté vacío
        LPC_UART0->THR = *str; // Enviar caracter
        str++; // Siguiente caracter
    }
}

/*=================================================================================*/
/*============================== ESTRUCTURA DE DATOS ==============================*/
/*=================================================================================*/

// ---- Crear un nuevo nodo ----
Nodo* crearNodo(Registro r) {
    Nodo* nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (!nuevo) return NULL;
    nuevo->data = r;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

// ---- Insertar nuevo registro en el árbol ----
// Si el ID ya existe, actualiza los datos.
Nodo* insertarNodo(Nodo* raiz, Registro r) {
    if (raiz == NULL) return crearNodo(r);

    if (r.id < raiz->data.id)
        raiz->izq = insertarNodo(raiz->izq, r);
    else if (r.id > raiz->data.id)
        raiz->der = insertarNodo(raiz->der, r);
    else
        raiz->data = r;  // Si ya existe, se actualizan los datos

    return raiz;
}

// ---- Buscar registro por ID ----
Nodo* buscarNodo(Nodo* raiz, uint32_t id) {
    if (raiz == NULL) return NULL;
    if (id == raiz->data.id) return raiz;
    if (id < raiz->data.id) return buscarNodo(raiz->izq, id);
    return buscarNodo(raiz->der, id);
}

// ---- Recorrer árbol (por ejemplo, para exportar por USB) ----
void recorrerInOrden(Nodo* raiz) {
    if (raiz == NULL) return;
    recorrerInOrden(raiz->izq);
    // printf("ID: %lu | Tipo: %d | Estado: %d | Cat: %d | Peso: %d kg\n",
    //        raiz->data.id, raiz->data.tipo, raiz->data.estado,
    //        raiz->data.categoria, raiz->data.pesoKg);
    recorrerInOrden(raiz->der);
}