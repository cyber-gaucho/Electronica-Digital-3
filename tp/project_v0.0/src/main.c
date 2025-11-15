/**
 * @file main.c
 * @brief Archivo principal del proyecto
 * @author Garcia Lautaro M
 * @author Renaudo G Valentino
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

uint16_t kilos;
system_state_t state;

int main(void) {

    // Inicialización general
    SystemInit();

    // Inicialización de módulos
    ui_init();
    buttons_init();       // Usa interrupciones (GPIOInt)
    adc_init();
    // dac_init();
    serial_init();
    // rfid_init();
    storage_init();

    // Estado inicial
    changeState(ST_BOOT);

    while (1) {
        // La lógica principal del sistema
        stateMachine();
    }
}