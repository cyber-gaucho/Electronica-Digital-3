/**
 * @file main.c
 * @brief Archivo principal del proyecto
 * @author Garcia Lautaro M
 * @author Renaudo G Valentino
 * @version 0.0.1
 * @copyright MIT License
 * @note Este archivo es el punto de entrada del proyecto
 */
/**
 * TODO:
 *  Revisar funciones storage
 *  Revisar funciones de save and send
 *  Mover ReadId de st_mch a id_reader.h/.c
 *  Agregar uitoa() a utils
 *  Agegar DAC con DMA
 *  DONE:
 *  Mover actualización de display a ui.c 
 *  Realizar delay por timer 2 con INT, Reset y Stop ENABLED
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
volatile uint8_t action; // Ver


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
    state = ST_BOOT;
    changeState(state);

    while (1) {
        // La lógica principal del sistema
        stateMachine();
    }
}