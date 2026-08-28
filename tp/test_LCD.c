#include 	"lpc17xx_i2c.h"
#include 	"lpc17xx_pinsel.h"
#include 	"lpc17xx_gpio.h"
#include 	"LiquidCrystal_I2C_LPC.h"
#include <cr_section_macros.h>

#define 	LCD_I2C_ADDR 	0x27
#define 	LCD_I2C_P		LPC_I2C0
#define 	RED_LED      	(1<<22)
// Configura LED ROJO
void cfgPin(void);

// Inicializa el periférico I2C
void cfgI2C0(void);
void test_I2C(void);

int main(void) {
	SystemInit();    // Inicializa el sistema y los relojes
	cfgPin();        
	cfgI2C0();        

    GPIO_ClearValue(0,RED_LED);	// P0.22 en bajo (enciende LED)
	lcd_init(LCD_I2C_ADDR);      // Inicializa el LCD

//	test_I2C();

	lcd_begin(LCD_I2C_P, 20, 4);
	lcd_clear();     // Limpia la pantalla
    lcd_setCursor(0, 0); lcd_print("      Grupo 1");
    lcd_setCursor(0, 1); lcd_print("  Garcia Lautaro M ");
    lcd_setCursor(0, 2); lcd_print(" Renaudo G Valentino");
    lcd_setCursor(0, 3); lcd_print("    Digital III");
    GPIO_ClearValue(0,RED_LED);	// P0.22 en bajo (enciende LED)

    while (1) {}
    return 0;
}


void cfgPin(){
    PINSEL_CFG_Type PinLED;
    PinLED.Portnum = 0;
    PinLED.Pinnum = 22; // LED ROJO
    PinLED.Funcnum = 0;
    PinLED.Pinmode = 0;
    PinLED.OpenDrain = 0;
	PINSEL_ConfigPin(&PinLED);

	GPIO_SetDir(0,RED_LED,1);	// P0.22 como salida
	GPIO_SetValue(0,RED_LED);	// P0.22 en alto (apaga LED)
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

void test_I2C(){
	I2C_M_SETUP_Type test_cfg;
	uint8_t patito = 0x00;
	test_cfg.sl_addr7bit = 0x27;
	test_cfg.tx_data = &patito;
	test_cfg.tx_length = 1;
	test_cfg.rx_data = NULL;
	test_cfg.rx_length = 0;
	test_cfg.retransmissions_max = 3;

	I2C_MasterTransferData(LPC_I2C0, &test_cfg, I2C_TRANSFER_POLLING);
}
