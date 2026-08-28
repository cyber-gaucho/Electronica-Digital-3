/**
 * @file LiquidCrystal_I2C_LPC.h
 * @brief Controlador para pantallas LCD basadas en el chip PCF8574 (I2C).
 *
 * Este módulo permite controlar pantallas LCD compatibles con el controlador
 * HD44780 a través de un expansor I2C tipo PCF8574. Ofrece funciones básicas
 * de inicialización, escritura y control del display.
 *
 * Conexión típica:
 *  - SCL → pin de reloj I2C
 *  - SDA → pin de datos I2C
 *  - VCC → 5V
 *  - GND → GND
 */

#ifndef LIQUIDCRYSTAL_I2C_LPC_H_
#define LIQUIDCRYSTAL_I2C_LPC_H_

#include <stdint.h>
#include <stdbool.h>
#include "LPC17xx.h"
#include "lpc17xx_i2c.h"

/** Dirección I2C por defecto del módulo PCF8574. */
#define LCD_I2C_ADDR_DEFAULT  0x27

/** Número de columnas del display. */
#define LCD_COLS_DEFAULT      16

/** Número de filas del display. */
#define LCD_ROWS_DEFAULT      2

// Pines del PCF8574
#define LCD_BACKLIGHT   0x08 // P3
#define LCD_ENABLE_BIT  0x04 // P2
#define LCD_RW          0X02 // P1
#define LCD_RS          0x01 // P0

/** Comandos del controlador HD44780 */
#define LCD_CMD_CLEAR_DISPLAY  0x01
#define LCD_CMD_RETURN_HOME    0x02
#define LCD_CMD_ENTRY_MODE     0x04
#define LCD_CMD_DISPLAY_CTRL   0x08
#define LCD_CMD_CURSOR_SHIFT   0x10
#define LCD_CMD_FUNCTION_SET   0x20
#define LCD_CMD_SET_CGRAM_ADDR 0x40
#define LCD_CMD_SET_DDRAM_ADDR 0x80

// ----- Flags para entry mode -----
#define LCD_ENTRYRIGHT  0x00
#define LCD_ENTRYLEFT   0x02
#define LCD_ENTRYSHIFTINCREMENT   0x01
#define LCD_ENTRYSHIFTDECREMENT   0x00

// ----- Flags para display control -----
#define LCD_DISPLAYON   0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON    0x02
#define LCD_CURSOROFF   0x00
#define LCD_BLINKON     0x01
#define LCD_BLINKOFF    0x00

// ----- Flags para function set -----
#define LCD_8BITMODE    0x10
#define LCD_4BITMODE    0x00
#define LCD_2LINE       0x08
#define LCD_1LINE       0x00
#define LCD_5x10DOTS    0x04
#define LCD_5x8DOTS     0x00

// variables definidas como estáticas en .c para que no sean globales
  // uint8_t lcd_i2cAddr; ///< Wire Address of the LCD
  // uint8_t lcd_backlight; ///< the backlight intensity
  // uint8_t lcd_cols; ///< number of cols of the display
  // uint8_t lcd_rows; ///< number of rows of the display
  // uint8_t lcd_entrymode; ///<flags from entrymode
  // uint8_t lcd_displaycontrol; ///<flags from displaycontrol
  // uint8_t lcd_row_offsets[4];

void lcd_init(uint8_t i2cAddr);

int lcd_begin(LPC_I2C_TypeDef *I2Cx, uint8_t cols, uint8_t rows);

/**
 * @brief Limpia el display completo.
 */
void lcd_clear(void);

/**
 * @brief Envía el cursor a la posición especificada.
 * @param col Columna (0 a n)
 * @param row Fila (0 a n)
 */
void lcd_setCursor(uint8_t col, uint8_t row);

/**
 * @brief Escribe una cadena completa de texto.
 * @param str Cadena terminada en nulo.
 */
void lcd_print(const char *str);

/**
 * @brief Activa o desactiva el backlight del LCD.
 * @param state true para encender, false para apagar.
 */
void lcd_backlight(bool state);

/**
 * @brief Desplaza el contenido del display a la izquierda.
 */
void lcd_scrollLeft(void);

/**
 * @brief Desplaza el contenido del display a la derecha.
 */
void lcd_scrollRight(void);

/**
 * TODO
 */
void lcd_displayOn();
void lcd_clearRow(uint8_t row);
void lcd_leftToRight();

#endif /* LIQUIDCRYSTAL_I2C_LPC_H_ */