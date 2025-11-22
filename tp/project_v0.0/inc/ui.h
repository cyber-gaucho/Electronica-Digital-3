/**
 * @file ui.h
 * @brief Interfaz de usuario
 * @author Garcia Lautaro M
 * @author Renaudo G Valentino
 * @date 2025-11-15
 * @version 0.0.1
 * @copyright MIT License
 * @note Este archivo es la interfaz de usuario del proyecto
*/
#ifndef UI_H
#define UI_H

#include 	"lpc17xx_i2c.h"
#include    "LiquidCrystal_I2C_LPC.h"

/* LCD Configuration */
#define LCD_I2C_ADDR    0x27
#define LCD_I2C_P       LPC_I2C0
#define LCD_WIDTH       20
#define LCD_HEIGHT      4

/**
 * @
 */
void ui_init(void);

/**
* @brief Displays the boot screen
*/
void ui_showBootScreen(void);

/**
* @brief Displays the wait ID screen
*/
void ui_showWaitIdScreen(void);

/**
* @brief Displays the read ID screen
*/
void ui_showReadIDScreen(char* id);

/**
* @brief Handles menu navigation in ST_MENU state
*/
void ui_showSavedScreen(char* id);

/**
* @brief Displays the send screen
*/
void ui_showSendScreen(char* guardados);

/**
* @brief Displays the NOT send screen
*/
void ui_showNotSendScreen(void);

void ui_printLine(uint8_t row, const char *text);
void ui_update(void);
void ui_update_kilos(uint16_t value);
void ui_update_menu(void);
void ui_update_menu_item(uint8_t item);
void ui_update_menu_item_value(uint8_t item, uint16_t value);
void ui_update_menu_item_value(uint8_t item, uint16_t value);

#endif