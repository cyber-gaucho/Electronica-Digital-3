/**
 * @file main.c
 * @brief Archivo principal del proyecto
 * @version 0.0.1
 * @copyright MIT License
 * @note Este archivo es el punto de entrada del proyecto
 */
/**
 * TODO:
 *  Revisar funciones storage
 *  Revisar funciones de save and send
 *  Agegar DAC con DMA
 * DONE:
 *  Mover actualización de display a ui.c 
 *  Realizar delay por timer 2 con INT, Reset y Stop ENABLED
 *  Mover ReadId de st_mch a id_reader.h/.c
 *  Agregar uitoa() a utils
 *  Agregar delay no bloqueante en menu
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
system_state_t state = ST_BOOT;
volatile uint8_t action = 0; // Ver


int main(void) {

    // Inicialización general
    SystemInit();
    
    // Inicialización de módulos
    ui_init();
    buttons_init();       // Usa interrupciones (GPIOInt)
    adc_init();
    // dac_init();  // revisar electrónica
    serial_init();
    id_init(ID_PREFIX, ID_START);
    // storage_init(); // placeholder

    // Estado inicial
    state = ST_BOOT;
    changeState(state);

    while (1) {
        // La lógica principal del sistema
        stateMachine();
    }
}