#include "state_machine.h"

// iNCLUDES EN .H
// #include "buttons.h"
// #include "ui.h"
// #include "storage.h"
// #include "adc.h"
// #include "LiquidCrystal_I2C_LPC.h"
// #include <stdint.h>
// #include <stdlib.h>
// #include <string.h>

// External variables
extern volatile uint8_t action;  // Button action from ISR
extern uint16_t kilos;           // Weight in kg from ADC

// Storage root node (global)
static Nodo* storage_root = NULL;

// Current ID being processed
static uint32_t current_id = 0;

// Menu variables (these should ideally be in a separate module, but for now we'll manage them here)
static uint8_t cursorIndex = 0;
static uint8_t itemSelection[3] = {0, 0, 0};

// Menu options (matching the structure from tp.c)
const char* itemNames[4] = {
    "Sexo  ",
    "Color ",
    "Categ ",
    "Peso  "
};

const char* options0[] = {"Hembra", "Macho", "-"};
const char* options1[] = {"Negro", "Colorado", "Careta", "Pampa"};
const char* options2[] = {"Ternero", "Vaquillona", "Novillo", "Toro", "Vaca", "Vaca Prenada"};

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
static uint32_t boot_delay_counter = 0;
#define BOOT_DELAY_MS 2000  // 2 seconds boot screen

// Saved screen delay counter
static uint32_t saved_delay_counter = 0;
#define SAVED_DELAY_MS 1500  // 1.5 seconds saved confirmation

/* Private Functions ---------------------------------------------------------- */

/**
* @brief Displays the boot screen
*/
static void showBootScreen(void) {
    lcd_clear();
    lcd_setCursor(3, 0);
    lcd_print("ED3 - Grupo 1");
    lcd_setCursor(2, 1);
    lcd_print("Garcia Lautaro M");
    lcd_setCursor(1, 2);
    lcd_print("Renaudo G Valentino");
    lcd_setCursor(2, 3);
    lcd_print("Registro ganadero");
}

/**
* @brief Displays the wait ID screen
*/
static void showWaitIdScreen(void) {
    lcd_clear();
    lcd_setCursor(3, 1);
    lcd_print("Esperando ID...");
    lcd_setCursor(3, 2);
    lcd_print("Presione ID btn");
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
                kilos_str[pos++] = '0' + (val / 100);
                val %= 100;
            }
            if (val >= 10 || pos > 0) {
                kilos_str[pos++] = '0' + (val / 10);
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
        lcd_setCursor(0, i);
        lcd_print(line);
    }
}

/**
* @brief Displays the saved confirmation screen
*/
static void showSavedScreen(void) {
    lcd_clear();
    lcd_setCursor(5, 1);
    lcd_print("Guardado!");
    lcd_setCursor(3, 2);
    lcd_print("ID: ");
    // Display ID (simplified - just show first few digits)
    char id_str[12];
    uint32_t id = current_id;
    uint8_t pos = 0;
    for (int i = 0; i < 8 && pos < 11; i++) {
        uint8_t digit = (id >> (28 - i*4)) & 0xF;
        id_str[pos++] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    id_str[pos] = '\0';
    lcd_print(id_str);
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
                showBootScreen();
                boot_delay_counter = 0;
                state_entry = 0;
            }
            
            // Wait for boot delay (simplified - in real implementation use timer)
            boot_delay_counter++;
            if (boot_delay_counter > (BOOT_DELAY_MS * 1000)) {  // Rough delay
                changeState(ST_WAIT_ID);
            }
            break;
            
        case ST_WAIT_ID:
            if (state_entry) {
                showWaitIdScreen();
                state_entry = 0;
            }
            
            // Check if ID button was pressed (action == 7)
            if (action == 7) {
                // Read ID
                current_id = readId();
                action = 0;  // Clear action
                
                // Check if ID already exists in storage
                Nodo* existing = buscarNodo(storage_root, current_id);
                if (existing != NULL) {
                    // Load existing data into menu
                    itemSelection[0] = existing->data.tipo;
                    itemSelection[1] = existing->data.estado;
                    itemSelection[2] = existing->data.categoria;
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
                showSavedScreen();
                saved_delay_counter = 0;
                state_entry = 0;

                // Save current menu data to the storage tree
                Animal data_to_save;
                data_to_save.id = current_id;
                data_to_save.tipo = itemSelection[0];
                data_to_save.estado = itemSelection[1];
                data_to_save.categoria = itemSelection[2];
                data_to_save.peso = kilos;  // Save the weight read by ADC

                storage_save(&storage_root, &data_to_save);
            }
            // Wait for saved delay
            saved_delay_counter++;
            if (saved_delay_counter > (SAVED_DELAY_MS * 1000)) {  // Rough delay
                // Check if we need to send after saving
                if (save_and_send) {
                    save_and_send = 0;  // Clear the flag
                    changeState(ST_SEND);
                } else {
                    // Transition back to wait ID state
                    changeState(ST_WAIT_ID);
                }
            }
            break;
        case ST_SEND:
            if (state_entry) {
                showSendScreen();
                send_delay_counter = 0;
                state_entry = 0;

                // Example: Send the current record via serial (UART)
                Nodo* toSend = storage_find(&storage_root, current_id);
                if (toSend != NULL) {
                    // Prepare and send a formatted string with ID and animal data
                    char buf[128];
                    snprintf(buf, sizeof(buf),
                        "ID:%lu, Sexo:%s, Color:%s, Categ:%s, Peso:%u\r\n",
                        toSend->data.id,
                        options0[toSend->data.tipo],
                        options1[toSend->data.estado],
                        options2[toSend->data.categoria],
                        toSend->data.peso
                    );
                    serial_send_string(buf);
                } else {
                    serial_send_string("No record to send\r\n");
                }
            }
            // Delay before returning to wait state
            send_delay_counter++;
            if (send_delay_counter > (SAVED_DELAY_MS * 1000)) {
                changeState(ST_WAIT_ID);
            }
            break;
    }
}
