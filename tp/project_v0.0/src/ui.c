#include "ui.h"

    void ui_init(void){

        I2C0_init();

        lcd_init(LCD_I2C_ADDR);
        lcd_begin(LCD_I2C_P, LCD_WIDTH, LCD_HEIGHT);
    }

    static I2C0_init(){
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

    void ui_printLine(uint8_t row, const char *text) {
        lcd_setCursor(0, row);
        lcd_print(text);
    }

    void ui_showBootScreen(void){
        lcd_clear();
        lcd_setCursor(3, 0);
        lcd_print("ED3 - Grupo 1");
        lcd_setCursor(2, 1);
        lcd_print("Garcia Lautaro M");
        lcd_setCursor(1, 2);
        lcd_print("Renaudo G Valentino");
        lcd_setCursor(2, 3);
        lcd_print("Registro ganadero");
    }

    void ui_showWaitIdScreen(void) {
        lcd_clear();
        lcd_setCursor(3, 1);
        lcd_print("Esperando ID...");
        lcd_setCursor(3, 2);
        lcd_print("Presione ID btn");
    }

    void ui_showSavedScreen(uint32_t id) {
        lcd_clear();
        lcd_setCursor(5, 1);
        lcd_print("Guardado!");
        lcd_setCursor(3, 2);
        lcd_print("ID: ");
        // Display ID (simplified - just show first few digits)
        char id_str[12];
        uint8_t pos = 0;
        for (int i = 0; i < 8 && pos < 11; i++) {
            uint8_t digit = (id >> (28 - i*4)) & 0xF;
            id_str[pos++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        }
        id_str[pos] = '\0';
        lcd_print(id_str);
    }

    void ui_showSendScreen(const char *guardados_str) {
        lcd_clear();
        lcd_setCursor(0, 1);
        lcd_print("Enviando ");
        lcd_print(guardados_str);
        lcd_print(" datos");
    }