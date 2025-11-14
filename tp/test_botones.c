#include 	"lpc17xx_i2c.h"
#include 	"lpc17xx_pinsel.h"
#include 	"lpc17xx_gpio.h"
#include 	"LiquidCrystal_I2C_LPC.h"

#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0

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
// #define BTN_ID_PIN      6
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

// Configura LEDs
void cfgPin(void);

// Inicializa el periférico I2C
void cfgI2C0(void);

void cfgGPIOINT(void);

volatile uint8_t action = 0;

void handleAction(void);

void setLED(uint8_t r, uint8_t g, uint8_t b);

int main(void) {
	SystemInit();    // Inicializa el sistema y los relojes
	cfgPin();        
    cfgGPIOINT();
	cfgI2C0();        
    
	lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD
    
	lcd_begin(LCD_I2C_P, 20, 4);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(6, 1); lcd_print("Grupo 1");
    lcd_setCursor(4, 2); lcd_print("Digital III");
    

    while (1) {
        handleAction();
    }
    return 0;
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

void cfgGPIOINT(){
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

    LPC_GPIOINT->IO2IntClr = 0xFFFF;
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
    if (LPC_GPIOINT->IO2IntStatF & (1 << BTN_ID_PIN)) { action = 7; LPC_GPIOINT->IO2IntClr = (1 << BTN_ID_PIN); }
}

/* --- Acciones --- */
void handleAction(void) {
    // static uint32_t lastActionTime = 0;
    if (action == 0) return;

    // if ((SysTick->VAL - lastActionTime) < 150) return;
    // lastActionTime = SysTick->VAL;

    switch (action) {
        case 1: // UP
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(5, 1); lcd_print("UP BUTTON");
            break;
        case 2: // DOWN
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("DOWN BUTTON");
            break;
        case 3: // RIGHT
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("RIGHT BUTTON");
            break;
        case 4: // LEFT
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("LEFT BUTTON");
            break;
        case 5: // SAVE
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("SAVE BUTTON");
            break;
        case 6: // SEND
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("SEND BUTTON");
            break;
        case 7: // ID
            lcd_clear();     // Limpia la pantalla
            lcd_setCursor(4, 1); lcd_print("ID BUTTON");
            break;
    }
    action = 0;
}

void setLED(uint8_t r, uint8_t g, uint8_t b){
    if(r) GPIO_ClearValue(0, 1<<LED_RED_PIN); else GPIO_SetValue(0, 1<<LED_RED_PIN);
    if(g) GPIO_ClearValue(3, 1<<LED_GREEN_PIN); else GPIO_SetValue(3, 1<<LED_GREEN_PIN);
    if(b) GPIO_ClearValue(3, 1<<LED_BLUE_PIN); else GPIO_SetValue(3, 1<<LED_BLUE_PIN);
}