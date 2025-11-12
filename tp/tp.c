/**
 * @file tp.c
 * @brief
 * 
 */

#include 	"lpc17xx_i2c.h"
#include 	"lpc17xx_pinsel.h"
#include 	"lpc17xx_gpio.h"
#include    "LiquidCrystal_I2C_LPC.h"
#include    <stdlib.h>
#include    <string.h>

// --- Display ---
#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0
#define     LCD_WIDTH       20
#define     LCD_HEIGHT      4

#define 	RED_LED_PIN     22

// --- Botones ---
#define     BTN_PORT        2
// #define BTN_UP_PIN      0
// #define BTN_DOWN_PIN    1
// #define BTN_RIGHT_PIN   2
// #define BTN_LEFT_PIN    3
// #define BTN_SAVE_PIN    4
// #define BTN_SEND_PIN    5
enum {
    BTN_UP_PIN = 0,
    BTN_DOWN_PIN, 
    BTN_RIGHT_PIN,
    BTN_LEFT_PIN,
    BTN_SAVE_PIN,
    BTN_SEND_PIN
} BTN_OPT;

volatile int action = 0;

// --- Pin analógico ---
#define LOAD_CELL_PORT  0
#define LOAD_CELL_PIN   23  // Entrada analógica (potenciómetro)

// --- PARÁMETROS ---
#define SERIAL_BAUD     9600
#define DEBOUNCE_MS     25
#define BLINK_INTERV_MS 400
#define UI_REFRESH_MS   100


/* Buffers */
#define BUFFER_SIZE 32
volatile uint16_t adcBuffer[BUFFER_SIZE];
volatile uint32_t adcBufferIndex = 0;
volatile bool new_sample_flag = false;


// -------- MENU / OPCIONES ----------
const char headerID[] = "0123456789ABCDE";

const char* itemNames[4] = {
  "Sexo ",
  "Color",
  "Categ",
  "Peso "
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
int cursorIndex = 0;  // 0..3 (el 3 es lectura, no configurable)

void SysTick_Handler(void);
void GPIO_IRQHandler(void);
void drawMenu(int peso);
void handleAction(void);
void setupButtons(void);
void cfgPin(void); // Otros pines como LED
void cfgI2C0(void);

int main(void) {
    SystemInit();
    SysTick_Config(SystemCoreClock / 1000); // 1 ms tick para debounce
        
	cfgI2C0();        

	lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD

	lcd_begin(LCD_I2C_P, LCD_WIDTH, LCD_HEIGHT);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(0, 0); lcd_print("      Grupo 1");
    lcd_setCursor(0, 1); lcd_print("  Garcia Lautaro M ");
    lcd_setCursor(0, 2); lcd_print(" Renaudo G Valentino");
    lcd_setCursor(0, 3); lcd_print("    Digital III");

    setupButtons();

    int peso = 250;
    while (1) {
        drawMenu(peso);
        handleAction();
    }
}
/*=================================================================================*/
/*==================================== Botones ====================================*/
/*=================================================================================*/

/**
 * @brief Configura pines e interrupciones para los botones
 * @todo: Actualizar con los pines seleccionados 
 */
void setupButtons(void) {
    LPC_GPIO0->FIODIR &= ~((1 << 18) | (1 << 11));
    LPC_GPIO2->FIODIR &= ~(1 << 13);

    LPC_GPIOINT->IO0IntEnF = (1 << 18) | (1 << 11);
    LPC_GPIOINT->IO2IntEnF = (1 << 13);
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

}

/* --- Lógica de menú --- */
void drawMenu(int peso) {
    lcd_clear();
    char buffer[21];
    for (int i = 0; i < 3; i++) {
        lcd_setCursor(0, i);
        if (i == cursorIndex) lcd_print(">");
        else lcd_print(" ");
        sprintf(buffer, "%s: %s", itemNames[i], itemOptions[i][itemSelection[i]]);
        lcd_print(buffer);
    }
    lcd_setCursor(0, 3);
    sprintf(buffer, "Peso: %d kg", peso);
    lcd_print(buffer);
}

/* --- Acciones --- */
void handleAction(void) {
    static uint32_t lastActionTime = 0;
    if (action == 0) return;

    if ((SysTick->VAL - lastActionTime) < 150) return;
    lastActionTime = SysTick->VAL;

    switch (action) {
        case 1: // UP
            if (cursorIndex > 0) cursorIndex--;
            break;
        case 2: // DOWN
            if (cursorIndex < 2) cursorIndex++;
            break;
        case 3: // OK
            itemSelection[cursorIndex]++;
            if (itemSelection[cursorIndex] > 2) itemSelection[cursorIndex] = 0;
            break;
    }
    action = 0;
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