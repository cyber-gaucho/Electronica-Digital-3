/**
 * @file LiquidCrystal_I2C_LPC.c
 * @brief Implementación de driver para LCD I2C con PCF8574 usando CMSIS.
 */

#include "E:\Electronica-Digital-3\tp\librerias\LiquidCrystal_I2C_LPC\inc\LiquidCrystal_I2C_LPC.h"
// #include "LiquidCrystal_I2C_LPC.h"

#include <string.h>
#include "system_LPC17xx.h"
// #include <util/delay.h>   // una función de delay

// Variables internas (contexto del LCD)
static LPC_I2C_TypeDef *lpc_i2c_p;
static uint8_t lcd_i2c_addr = LCD_I2C_ADDR_DEFAULT;
static uint8_t backlight_state = LCD_BACKLIGHT;
static uint8_t lcd_cols = LCD_COLS_DEFAULT;
static uint8_t lcd_rows = LCD_ROWS_DEFAULT;
static uint8_t lcd_displaycontrol;
static uint8_t lcd_entrymode;
static uint8_t lcd_row_offsets[4];

/* ---------- Rutinas básicas de I2C (CMSIS) ---------- */

static void i2c_writeByte(uint8_t data) {
    uint8_t _data = data; 
    I2C_M_SETUP_Type cfg;
    cfg.sl_addr7bit = lcd_i2c_addr;
    cfg.tx_data = &_data;
    cfg.tx_length = 1;
    cfg.rx_data = NULL;
    cfg.rx_length = 0;
    cfg.retransmissions_max = 3;

    I2C_MasterTransferData(lpc_i2c_p, &cfg, I2C_TRANSFER_POLLING);
}

// static void lcd_delayUs(uint32_t us) {
//     uint32_t freq = SystemCoreClock / 1000000;  // ciclos por microsegundo
//     for (volatile uint32_t i = 0; i < (us * freq / 5); i++) {
//         __NOP();
//     }
// }
static void lcd_delayUs(uint32_t us) {
    uint32_t cycles = (SystemCoreClock / 1000000) * us;
    for (volatile uint32_t i = 0; i < cycles; i++) {
        __NOP();
    }
}

/* ---------- Nivel lógico del LCD ---------- */

static void lcd_sendNibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble << 4) | backlight_state | mode;     // mode es el bit RS
    
    i2c_writeByte( data | LCD_ENABLE_BIT);
    lcd_delayUs(1);
    
    i2c_writeByte( data & ~LCD_ENABLE_BIT);
    lcd_delayUs(50);
}

static void lcd_sendByte(uint8_t value, uint8_t mode) {
    lcd_sendNibble((value >> 4) & 0x0F, mode);
    lcd_sendNibble(value & 0x0F, mode);
}

static void lcd_command(uint8_t value) {
    lcd_sendByte(value, 0); // RS = 0 para comando
}

static void lcd_writeChar(char ch) {
    lcd_sendByte(ch, 1);    // RS = 1 para dato
}

/* ---------- API pública ---------- */

void lcd_init(uint8_t i2cAddr) {
    lcd_i2c_addr = i2cAddr;
}

int lcd_begin(LPC_I2C_TypeDef *I2Cx, uint8_t cols, uint8_t rows) {
    lpc_i2c_p = I2Cx;   // Usa el puerto seleccionado por el ususario

    lcd_cols = (cols > 80) ? 80 : cols;
    lcd_rows = (rows > 4) ? 4 : rows;
    
    uint8_t functionFlags = 0;

    lcd_row_offsets[0] = 0x00;
    lcd_row_offsets[1] = 0x40;
    lcd_row_offsets[2] = 0x00 + cols;
    lcd_row_offsets[3] = 0x40 + cols;

    if (rows > 1) functionFlags |= LCD_2LINE;

    lcd_displaycontrol = LCD_DISPLAYON;
    lcd_entrymode = LCD_ENTRYLEFT;

    // El bus I2C ya debe estar inicializado externamente.
    i2c_writeByte(0x00 | LCD_BACKLIGHT);
    lcd_delayUs(50000);

    // Secuencia de inicialización HD44780 (modo 4 bits)
    lcd_sendNibble(0x03, 0);
    lcd_delayUs(4500);
    lcd_sendNibble(0x03, 0);
    lcd_delayUs(200);
    lcd_sendNibble(0x03, 0);
    lcd_delayUs(200);
    lcd_sendNibble(0x02, 0); // finally, set to 4-bit interface

    lcd_command(LCD_CMD_FUNCTION_SET | functionFlags);

    // Llamadas equivalentes a display(), clear(), leftToRight()
    lcd_displayOn();
    lcd_clear();
    lcd_leftToRight();
    return 1; // el display se pudo inicializar bien
}

void lcd_clear(void) {
    lcd_sendByte(LCD_CMD_CLEAR_DISPLAY, 0);
    lcd_delayUs(2000);
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    if ((col < lcd_cols) && (row < lcd_rows)) {
        lcd_sendByte(LCD_CMD_SET_DDRAM_ADDR | (col + lcd_row_offsets[row]), 0);
    }
}

void lcd_print(const char *str) {
    while (*str) lcd_writeChar(*str++);
}

void lcd_backlight(bool state) {
    backlight_state = state ? LCD_BACKLIGHT : 0x00;
    lcd_command(backlight_state);
}

void lcd_scrollLeft(void) {
    lcd_sendByte(0x18, 0);
}

void lcd_scrollRight(void) {
    lcd_command(0x1C);
}

// Enciende el display (sin alterar cursor o blink)
void lcd_displayOn(void) {
    lcd_displaycontrol |= LCD_DISPLAYON;
    lcd_command(LCD_CMD_DISPLAY_CTRL | lcd_displaycontrol);
}

// Configura dirección de texto de izquierda a derecha
void lcd_leftToRight(void) {
    lcd_entrymode |= LCD_ENTRYLEFT;
    lcd_command(LCD_CMD_ENTRY_MODE | lcd_entrymode);
}

// Borra una fila específica
void lcd_clearRow(uint8_t row) {
    if (row >= lcd_rows) return;
    lcd_setCursor(0, row);
    for (uint8_t i = 0; i < lcd_cols; i++) {
        lcd_writeChar(' ');
    }
    lcd_setCursor(0, row);
}