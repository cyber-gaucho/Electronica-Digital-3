#ifndef STATE_MACHINE
#define STATE_MACHINE

typedef enum {
    ST_BOOT,          // Pantalla inicial
    ST_WAIT_ID,       // Esperando llegada del ID
    ST_MENU,          // Menú cuando llegó un ID
    ST_SAVED,         // Dato guardado, volver a esperar otro ID
    ST_SEND
} system_state_t;

extern system_state_t state;

void changeState(system_state_t NewState);
void stateMachine(void);

#endif