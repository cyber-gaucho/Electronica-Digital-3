#include "E:\Electronica-Digital-3\common\cmsis\CMSISv2p00_LPC17xx\Drivers\inc\lpc17xx_pinsel.h"
#include "E:\Electronica-Digital-3\common\cmsis\CMSISv2p00_LPC17xx\Drivers\inc\lpc17xx_gpio.h"

/************************** PRIVATE DEFINITIONS *************************/
/* Switch pins */
#define SWITCH1_PORT    0
#define SWITCH1_PIN     2       // P0.2 para Switch 1
#define SWITCH2_PORT    0
#define SWITCH2_PIN     3       // P0.3 para Switch 2

/* LED pins - LEDs integrados en la placa */
#define LED1_PORT       0
#define LED1_PIN        22      // P0.22 LED i
#define LED2_PORT       3
#define LED2_PIN        25      // P3.25 LED
#define LED3_PORT       3
#define LED3_PIN        26      // P3.26 LED

/************************** PRIVATE VARIABLES *************************/
/* Operation modes */
typedef enum {
    MODE_CHANNEL1 = 0,    // Solo canal 1 (switches en 00)
    MODE_CHANNEL2,        // Solo canal 2 (switches en 01)
    MODE_SUM,             // Suma CH1 + CH2 (switches en 10)
    MODE_INVERT           // Inversión de CH1 (switches en 11)
} operation_mode_t;

/************************** FUNCTIONS *************************/
operation_mode_t ReadSwitches(void) {
    // Leer estado de los switches
    // Con pull-up interno: 1 = switch abierto (no presionado), 0 = switch cerrado (presionado)
    uint8_t switch1_pressed = !(GPIO_ReadValue(SWITCH1_PORT) & (1 << SWITCH1_PIN));
    uint8_t switch2_pressed = !(GPIO_ReadValue(SWITCH2_PORT) & (1 << SWITCH2_PIN));



    if (!switch1_pressed && !switch2_pressed) {
        return MODE_CHANNEL1;     // Modo por defecto
    } else if (switch1_pressed && !switch2_pressed) {
        return MODE_CHANNEL2;     // Solo SW1
    } else if (!switch1_pressed && switch2_pressed) {
        return MODE_SUM;          // Solo SW2
    } else {
        return MODE_INVERT;       // Ambos switches
    }
}

void UpdateLEDs(operation_mode_t mode) {
    // Primero apagar todos los LEDs
    GPIO_ClearValue(LED1_PORT, (1 << LED1_PIN));
    GPIO_ClearValue(LED2_PORT, (1 << LED2_PIN));
    GPIO_ClearValue(LED3_PORT, (1 << LED3_PIN));

    // Encender LED según el modo activo
    switch (mode) {
        case MODE_CHANNEL1:
            // LED1 ON - Canal 1 (modo por defecto)
            GPIO_SetValue(LED1_PORT, (1 << LED1_PIN));
            break;

        case MODE_CHANNEL2:
            // LED2 ON - Canal 2
            GPIO_SetValue(LED2_PORT, (1 << LED2_PIN));
            break;

        case MODE_SUM:
            // LED3 ON - Modo SUMA
            GPIO_SetValue(LED3_PORT, (1 << LED3_PIN));
            break;

        case MODE_INVERT:
            // LED1 y LED2 ON - Modo INVERSIÓN
            GPIO_SetValue(LED1_PORT, (1 << LED1_PIN));
            GPIO_SetValue(LED2_PORT, (1 << LED2_PIN));
            break;

        default:
            // Por seguridad, modo por defecto
            GPIO_SetValue(LED1_PORT, (1 << LED1_PIN));
            break;
    }
}

void configGPIO(void) {
    PINSEL_CFG_Type PinCfg;

    /* Switches configuration como entradas con pull-up interno */
    PinCfg.Funcnum = PINSEL_FUNC_0;      // Función GPIO
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_PULLUP;  // Pull-up interno habilitado
    PinCfg.Portnum = PINSEL_PORT_0;

    // Configurar P0.2 como entrada con pull-up (Switch 1)
    PinCfg.Pinnum = SWITCH1_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(SWITCH1_PORT, (1 << SWITCH1_PIN), 0);  // Configurar como entrada

    // Configurar P0.3 como entrada con pull-up (Switch 2)
    PinCfg.Pinnum = SWITCH2_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(SWITCH2_PORT, (1 << SWITCH2_PIN), 0);  // Configurar como entrada

    /* LEDs configuration como salidas */
    // LED1 en P0.22
    PinCfg.Funcnum = PINSEL_FUNC_0;      // Función GPIO
    PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
    PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
    PinCfg.Portnum = PINSEL_PORT_0;
    PinCfg.Pinnum = LED1_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LED1_PORT, (1 << LED1_PIN), 1);  // Output
    GPIO_ClearValue(LED1_PORT, (1 << LED1_PIN));

    // LED2 en P3.25
    PinCfg.Portnum = PINSEL_PORT_3;
    PinCfg.Pinnum = LED2_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LED2_PORT, (1 << LED2_PIN), 1);  // Output
    GPIO_ClearValue(LED2_PORT, (1 << LED2_PIN));

    // LED3 en P3.26
    PinCfg.Pinnum = LED3_PIN;
    PINSEL_ConfigPin(&PinCfg);
    GPIO_SetDir(LED3_PORT, (1 << LED3_PIN), 1);  // Output
    GPIO_ClearValue(LED3_PORT, (1 << LED3_PIN));

    /* Test inicial - encender todos los LEDs por un tiempo */
    GPIO_SetValue(LED1_PORT, (1 << LED1_PIN));
    GPIO_SetValue(LED2_PORT, (1 << LED2_PIN));
    GPIO_SetValue(LED3_PORT, (1 << LED3_PIN));

    // Delay de prueba
    for(volatile int i = 0; i < 1000000; i++);

    // Apagar todos
    GPIO_ClearValue(LED1_PORT, (1 << LED1_PIN));
    GPIO_ClearValue(LED2_PORT, (1 << LED2_PIN));
    GPIO_ClearValue(LED3_PORT, (1 << LED3_PIN));


}

int main(void) {
    configGPIO();


    operation_mode_t current_mode;


    while (1) {
        // Leer estado de switches
        current_mode = ReadSwitches();

        // Actualizar LEDs según el modo
        UpdateLEDs(current_mode);

        // Pequeño delay para estabilidad
        for(volatile int i = 0; i < 50000; i++);
    }

    return 0;
}



