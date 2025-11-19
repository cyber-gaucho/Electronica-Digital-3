#include "state_machine.h"

#include "adc.h"
#include "buttons.h"
#include "storage.h"
#include "ui.h"
#include "utils.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/** Esto includes estan en el .c ya que NO son necesarios para las
 *  declaraciones publicas. Solo se necesitan para la implementación */

// External variables
extern volatile uint8_t action;  // Button action from ISR
extern uint16_t kilos;           // Weight in kg from ADC

// Storage root node (global)
static Nodo* storage_root = NULL;

// Current ID being processed
static uint32_t current_id = 0;
// Total count of saved registers
static uint32_t guardados = 0;

// Menu variables (these should ideally be in a separate module, but for now we'll manage them here)
static uint8_t cursorIndex = 0;
static uint8_t itemSelection[3] = {0, 0, 0};

// Menu options (matching the structure from tp.c)
const char* itemNames[4] = {
    "Raza   ",
    "Categ  ",
    "Origen ",
    "Peso   "
};

const char* options0[] = {"Angus", "Hereford", "Holando", "Brangus", "Braford", "Shorthorn", "Charolais"};
const char* options1[] = {"Ternero/a", "Novillo", "Vaquillona", "Vaca", "Toro"};
const char* options2[] = {"Los Cardales", "La Soñada", "El Algarrobo", "La Tranquera"};

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

// Boot screen delay counter
#define BOOT_DELAY_MS 2000  // 2 seconds boot screen
// Saved screen delay counter
#define SAVED_DELAY_MS 2000  // 2 seconds saved confirmation
// Saved screen delay counter
#define SEND_DELAY_MS 2000

/* Private Functions ---------------------------------------------------------- */

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
                kilos_str[pos++] = '0' + (val / 10);    // Carga centenas en pos
                val %= 10;
            }
            kilos_str[pos++] = '0' + val;
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
            else cursorIndex = 3;
            break;
            
        case 2: // DOWN
            if (cursorIndex < 3) cursorIndex++;
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
        // Save the record
            {
            Registro reg;
            reg.id = current_id;
            reg.tipo = itemSelection[0];      // Sexo
            reg.estado = itemSelection[1];    // Color
            reg.categoria = itemSelection[2]; // Categ
            reg.pesoKg = (uint16_t)kilos;
            
            // Insert into storage tree
            storage_root = insertarNodo(storage_root, reg);
            
            // Reset menu selections
            cursorIndex = 0;
            itemSelection[0] = 0;
            itemSelection[1] = 0;
            itemSelection[2] = 0;
                
            // Transition to saved state
            changeState(ST_SAVED);
            }
            break;
            
        case 6: // SEND (optional - could export data)
            // Set flag to save and then send
            save_and_send = 1;
            // Save the record first
            {
            Registro reg;
            reg.id = current_id;
            reg.tipo = itemSelection[0];      // Sexo
            reg.estado = itemSelection[1];    // Color
            reg.categoria = itemSelection[2]; // Categ
            reg.pesoKg = (int)kilos;
            
            // Insert into storage tree
            storage_root = insertarNodo(storage_root, reg);
            
            // Reset menu selections
            cursorIndex = 0;
            itemSelection[0] = 0;
            itemSelection[1] = 0;
            itemSelection[2] = 0;
                
            // Transition to saved state (which will then go to send state)
            changeState(ST_SAVED);
            }
            break;
            
        case 7: // ID (ignore in menu state)
            break;
        }
        
    action = 0;  // Clear action
}

/**
* @brief Simulates reading an ID (for now, just generates a test ID)
* In the future, this should call id_reader functions
*/
static uint32_t readId(void) {
    // For now, generate a simple test ID
    // In real implementation, this would call id_reader functions
    static uint32_t test_id_counter = 1;
    return test_id_counter++;
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
    switch (state) {
        case ST_BOOT:
            if (state_entry) {
                ui_showBootScreen();
                state_entry = 0;
            }
            delayTIM2(BOOT_DELAY_MS);
            changeState(ST_WAIT_ID);            
            break;
            
        case ST_WAIT_ID:
            if (state_entry) {
                showWaitIdScreen();
                state_entry = 0;
            }
            // ADD: Check if SEND button was pressed (action == 6)
            // Check if ID button was pressed (action == 7)
            if (action == 7) {
                // Read ID
                current_id = readId();
                action = 0;  // Clear action
                
                // Check if ID already exists in storage
                Nodo* existing = buscarNodo(storage_root, current_id);
                if (existing != NULL) {
                    // Load existing data into menu
                    itemSelection[0] = existing->data.raza;
                    itemSelection[1] = existing->data.categoria;
                    itemSelection[2] = existing->data.origen;
                } else {
                    // Reset menu selections for new ID
                    cursorIndex = 0;
                    itemSelection[0] = 0;
                    itemSelection[1] = 0;
                    itemSelection[2] = 0;
                }
                
                // Transition to menu state
                changeState(ST_MENU);
            }
            break;
            
        case ST_MENU:
            if (state_entry) {
                showMenuScreen();
                state_entry = 0;
            }
            
            // Handle menu navigation
            handleMenuNavigation();
            
            // Update menu display periodically
            // (In a real implementation, use a timer to avoid constant updates)
            static uint32_t menu_update_counter = 0;
            menu_update_counter++;
            if (menu_update_counter > 10000) {  // Rough periodic update
                showMenuScreen();
                menu_update_counter = 0;
            }
            break;
            
        case ST_SAVED:
            if (state_entry) {
                ui_showSavedScreen(current_id);
                state_entry = 0;

                // Save current menu data to the storage tree
                Registro r;
                r.id = current_id;
                r.raza = itemSelection[0];
                r.categoria = itemSelection[1];
                r.origen = itemSelection[2];
                r.pesoKg = kilos;  // Save the weight read by ADC

                storage_save(&storage_root, &r);
            }
            
            delayTIM2(SAVED_DELAY_MS);
            if (save_and_send) {
                save_and_send = 0;  // Clear the flag
                changeState(ST_SEND);
            } else {
                // Transition back to wait ID state
                changeState(ST_WAIT_ID);
            }
            break;
        case ST_SEND:
            if (state_entry) {
                state_entry = 0;
                if(guardados > 0){
                    const char* guardados_str;
                    uitoa(guardados, guardados_str, 10);
                    ui_showSendScreen(guardados_str);

                } else {
                    ui_showNotSendScreen();
                }

                // // Example: Send the current record via serial (UART)
                // Nodo* toSend = storage_find(&storage_root, current_id);
                // if (toSend != NULL) {
                //     // Prepare and send a formatted string with ID and animal data
                //     char buf[128];
                //     snprintf(buf, sizeof(buf),
                //         "ID:%lu, Raza:%s, Categ:%s, Origen:%s, Peso:%u\r\n",
                //         toSend->data.id,
                //         options0[toSend->data.raza],
                //         options1[toSend->data.categoria],
                //         options2[toSend->data.origen],
                //         toSend->data.pesoKg
                //     );
                //     serial_send_string(buf);
                // } else {
                //     serial_send_string("No record to send\r\n");
                // }
            }
            // Delay before returning to wait state
            delayTIM2(SEND_DELAY_MS);
            break;
    }
}
