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