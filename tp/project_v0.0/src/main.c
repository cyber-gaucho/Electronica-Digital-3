/**
 * @file main.c
 * @brief Archivo principal del proyecto
 * @version 0.0.1
 * @copyright MIT License
 * @note Este archivo es el punto de entrada del proyecto
 */
#include "lpc17xx.h"

// Define serial communication method before including serial.h
#define SERIAL_UART  // Use UART for serial communication
// #define SERIAL_USB  // Uncomment to use USB instead

#include "adc.h"
#include "buttons.h"
#include "dac.h"
#include "id_reader.h"
#include "serial.h"
#include "state_machine.h"
#include "storage.h"
#include "ui.h"
#include "utils.h"

#define ID_PREFIX   123 // Prefix for ID generation
#define ID_START    0

volatile uint32_t ticksMs = 0;
uint16_t kilos = 0;
system_state_t estado;
volatile uint8_t action = 0;


int main(void) {
    // Inicialización general
	SystemInit();

    // Inicialización de módulos
    utils_init();
    ui_init();
    adc_init();
    buttons_init();       // Usa interrupciones (GPIOInt)
    dac_init();
    serial_init();
    id_init(ID_PREFIX, ID_START);
    storage_init();

    // Estado inicial
    estado = ST_BOOT;
    changeState(estado);

    while (1) {
        // La lógica principal del sistema
        stateMachine();
    }
}
