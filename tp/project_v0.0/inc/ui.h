#ifndef UI_H
#define UI_H

/* LCD Configuration */
#define LCD_I2C_ADDR    0x27
#define LCD_I2C_P       LPC_I2C0
#define LCD_WIDTH       20
#define LCD_HEIGHT      4

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

#endif