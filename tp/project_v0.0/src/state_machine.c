#include "state_machine.h"
#include "lpc17xx_adc.h"

#include "adc.h"
#include "buttons.h"
#include "dac.h"
#include "id_reader.h"
#include "serial.h"
#include "storage.h"
#include "ui.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Esto includes estan en el .c ya que NO son necesarios para las
 * declaraciones publicas. Solo se necesitan para la implementación */

// External variables
extern volatile uint8_t action;  // Button action from ISR
extern uint16_t kilos;           // Weight in kg from ADC

// Current ID being processed
static uint64_t current_id = 0;
char id_str[15];
// Total count of saved registers
static uint32_t guardados = 0;
char guardados_str[4];

// Menu variables (these should ideally be in a separate module, but for now we'll manage them here)
static uint8_t cursorIndex = 0;
static uint8_t itemSelection[3] = {0, 0, 0};

// Menu options
const char* itemNames[4] = {
    "Raza  ",
    "Categ ",
    "Origen",
    "Peso  "
};

const char* options0[] = {"Angus", "Hereford", "Holando", "Brangus", "Braford", "Shorthorn", "Charolais"};
const char* options1[] = {"Ternero/a", "Novillo", "Vaquillona", "Vaca", "Toro"};
const char* options2[] = {"Los Cardales", "Los Medanos", "El Algarrobo", "La Tranquera"};

const char** itemOptions[3] = {options0, options1, options2};
const uint8_t itemOptionsCount[3] = {
    sizeof(options0) / sizeof(options0[0]),
    sizeof(options1) / sizeof(options1[0]),
    sizeof(options2) / sizeof(options2[0])
};

// State entry flag
static uint8_t state_entry = 1;

// Flag to indicate save and send operation
static uint8_t save_and_send = 0;

// Boot screen delay time
#define BOOT_DELAY_MS 2000
// Saved screen delay time
#define SAVED_DELAY_MS 2000
// Send screen delay time
#define SEND_DELAY_MS 2000
// ID screen delay time
#define ID_DELAY_MS 2000
// Menu update time
#define MENU_UPDATE_MS 200

/* Private Functions ---------------------------------------------------------- */

static void export_registro_csv(const Registro* r) {
    char buffer[20];

    // ID (uint64)
    utils_uitoa(r->id, buffer, 10);
    serial_send_string(buffer);
    serial_send_string(",");

    // Raza → string desde options0[]
    serial_send_string(options0[r->raza]);
    serial_send_string(",");

    // Categoría → string desde options1[]
    serial_send_string(options1[r->categoria]);
    serial_send_string(",");

    // Origen → string desde options2[]
    serial_send_string(options2[r->origen]);
    serial_send_string(",");

    // Peso (uint16)
    utils_uitoa(r->pesoKg, buffer, 10);
    serial_send_string(buffer);

    // Fin de línea
    serial_send_string("\r\n");
}


static void exportar_todos_los_registros(void) {
    uint16_t count = guardados;

    for (uint16_t i = 0; i < count; i++) {
        Registro* r = buscarRegistroIndex(i);
        if (r != NULL) {
            export_registro_csv(r);
        }
    }
}


/**
* @brief Displays the menu screen
*/
static void showMenuScreen(void) {
    char line[21];
    uint8_t i, j;
    const char* src;
    
    for (i = 0; i < 4; i++) {
        // Clear line buffer
        for (j = 0; j < 20; j++) line[j] = ' ';
        line[20] = '\0';
        
        j = 0;
        // Selection indicator
        line[j++] = (i == cursorIndex) ? '>' : ' ';
        
        // Item name
        src = itemNames[i];
        while (*src && j < 20) line[j++] = *src++;
        
        if (j < 20) line[j++] = ' ';
        
        // Item value
        if (i < 3) {
            src = itemOptions[i][itemSelection[i]];
        } else {
            // Weight - convert kilos to string
            char kilos_str[6];
            uint16_t val = kilos;
            uint8_t pos = 0;
            if (val >= 100) {
                kilos_str[pos++] = '0' + (val / 100);   // Carga centenas en pos
                val %= 100;
            }
            if (val >= 10 || pos > 0) {
                kilos_str[pos++] = '0' + (val / 10);    // Carga decenas en pos
                val %= 10;
            }
            kilos_str[pos++] = '0' + val; // Carga unidades
            kilos_str[pos++] = ' ';
            kilos_str[pos++] = 'k';
            kilos_str[pos++] = 'g';
            kilos_str[pos] = '\0';
            src = kilos_str;
        }
        
        while (*src && j < 20) line[j++] = *src++;
        
        // Display line
        ui_printLine(i, line);
    }
}

/**
* @brief Handles menu navigation in ST_MENU state
*/
static void handleMenuNavigation(void) {
    if (action == 0) return;
    
    switch (action) {
        case 1: // UP
            if (cursorIndex > 0) cursorIndex--;
            else cursorIndex = 2;
            break;
            
        case 2: // DOWN
            if (cursorIndex < 2) cursorIndex++;
            else cursorIndex = 0;
            break;
            
        case 3: // RIGHT
            if (cursorIndex < 3) {
                itemSelection[cursorIndex]++;
                if (itemSelection[cursorIndex] >= itemOptionsCount[cursorIndex]) {
                    itemSelection[cursorIndex] = 0;
                }
            }
            break;
            
        case 4: // LEFT
            if (cursorIndex < 3) {
                if (itemSelection[cursorIndex] == 0) {
                    itemSelection[cursorIndex] = itemOptionsCount[cursorIndex] - 1;
                } else {
                    itemSelection[cursorIndex]--;
                }
            }
            break;
            
        case 5: // SAVE
            // Transition to saved state
            changeState(ST_SAVED);
            break;
            
        case 6: // SEND
            // Set flag to save and then send
            // Save the record first
            save_and_send = 1;
            // Transition to saved state (which will then go to send state)
            changeState(ST_SAVED);
            break;
            
        case 7: // ID (ignore in menu state)
            break;
        }
        
    action = 0;  // Clear action
}

/* End of Private Functions --------------------------------------------------- */

/**
 * @brief Changes the system state
 * @param NewState The new state to transition to
 */
void changeState(system_state_t NewState) {
    state = NewState;
    state_entry = 1;  // Mark that we just entered this state
}

/**
* @brief Main state machine function
* This function is called continuously from main loop
*/
void stateMachine() {
    switch (estado) {
        case ST_BOOT:
        	static uint32_t boot_delay = 0;
            if (state_entry == 1) {
				boot_delay = ticksMs + BOOT_DELAY_MS;
                ui_showBootScreen();
                state_entry = 0;
            }
//            delayTIM2(BOOT_DELAY_MS);
            if ((int32_t)(ticksMs - boot_delay) >= 0) {
				 changeState(ST_WAIT_ID);
			}
            break;
            
        case ST_WAIT_ID:
            if (state_entry) {
                ui_showWaitIdScreen();
                state_entry = 0;
            }
            // ADD: Check if SEND button was pressed (action == 6)
            // Check if ID button was pressed (action == 7)
            while (!(action == 6 || action == 7)) {};
            if (action == 7) {
                // Read ID
                current_id = id_generate();
                action = 0;  // Clear action
                
                utils_uitoa(current_id, id_str, 10);
                ui_showReadIDScreen(id_str);

                generateTone(440, 1000);

                delayTIM2(ID_DELAY_MS);	// Funciona!!!
                changeState(ST_MENU);
            }
            else if (action == 6) {
                action = 0;  // Clear action
                changeState(ST_SEND);
            }
            break;
            
        case ST_MENU:
            static uint32_t next_update = 0;
            if (state_entry) {
                showMenuScreen();
                state_entry = 0;
				next_update = ticksMs + MENU_UPDATE_MS;
            }
            
            handleMenuNavigation();
            
            if ((int32_t)(ticksMs - next_update) >= 0) {
            	kilos = adc_getkilos();
                showMenuScreen();
                next_update = ticksMs + MENU_UPDATE_MS;
            }
            break;
            
        case ST_SAVED:
            if (state_entry) {
                state_entry = 0;
                
                storage_guardarDato(current_id, itemSelection[0],
                		itemSelection[1], itemSelection[2], kilos);
                
                cursorIndex = 0;
                // itemSelection[0] = 0;
                // itemSelection[1] = 0;
                // itemSelection[2] = 0;
                guardados++;
                ui_showSavedScreen(id_str);
            }
            
            delayTIM2(SAVED_DELAY_MS);
            
            if (save_and_send == 1) {
                save_and_send = 0;
                changeState(ST_SEND);
            } else {
                changeState(ST_WAIT_ID);
            }
            break;

        case ST_SEND:
            if (state_entry) {
                state_entry = 0;
                if(guardados > 0){
                    utils_uitoa(guardados, guardados_str, 10);
                    ui_showSendScreen(guardados_str);
                    exportar_todos_los_registros();
                } else {
                    ui_showNotSendScreen();
                }

            }
            delayTIM2(SEND_DELAY_MS);
            changeState(ST_WAIT_ID);
            break;
    }
}
