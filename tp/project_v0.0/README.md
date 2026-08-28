# Proyecto Integrador – LPC176x

Este proyecto implementa un sistema embebido sobre un microcontrolador **LPC176x** que integra:

* Lectura y gestión de identificaciones RFID (simuladas inicialmente).
* Interfaz de usuario con **LCD 20×4 vía I2C**.
* Navegación mediante botones con antirrebote e interrupciones.
* Adquisición de peso mediante **ADC con ISR**.
* Reproducción de sonidos mediante **DAC + DMA**.
* Almacenamiento eficiente de datos (árbol binario).
* Comunicación con PC mediante **UART o USB-CDC**.
* Máquina de estados para organizar el flujo del sistema.

El código está organizado en módulos independientes para mejorar claridad, mantenibilidad y escalabilidad.

---

## Árbol del proyecto

```
/project
    /src
        main.c
        state_machine.c
        ui.c
        buttons.c
        adc.c
        dac.c
        id_reader.c
        storage.c
        serial.c
        utils.c

    /include
        state_machine.h
        ui.h
        buttons.h
        adc.h
        dac.h
        id_reader.h
        storage.h
        serial.h
        utils.h

    README.md
```

---

## Descripción de cada módulo

### 1. **state_machine.c**

Implementa la lógica general mediante un `enum` de estados:

* Pantalla inicial
* Espera de lectura
* Configuración de datos
* Guardado
* Retorno al estado inicial

Funciones principales:

* `stateMachine()` → ejecuta el comportamiento del estado actual.
* `changeState()` → configura módulos/periféricos al cambiar de estado.

### 2. **ui.c** – Interfaz LCD

Maneja el display 20×4:

* `ui_init()`
* `ui_showBoot()`
* `ui_showMenu()`
* `ui_updateWeight()`
* `ui_showSaved()`

Usa buffers de líneas y actualizaciones diferenciales para minimizar parpadeo.

### 3. **buttons.c** – Botones y antirrebote

Se encarga de:

* Antirrebote por software.
* Interrupciones por flanco.
* Reportar eventos a la máquina de estados.

### 4. **adc.c** – Lectura del peso

Configura ADC + interrupción:

* ISR captura valor ADC.
* Se convierte el valor a kilogramos.
* Actualiza un string para el LCD.

Variables como `kilos` y `kilos_str` son globales con `extern` en el header.

### 5. **dac.c** – Sonido con DAC + DMA

Genera tonos de buzzer:

* Se carga un buffer de muestra (onda senoidal o cuadrada).
* DMA transfiere datos al DAC periódicamente.
* Funciones:

  * `dac_init()`
  * `dac_playTone(freq, dur_ms)`
  * `dac_stop()`

Esto permite reproducir sonidos sin bloquear la CPU.

### 6. **id_reader.c** – Lectura de ID

Mientras no se use RFID real, genera IDs simulados:

* Por botón o trigger externo.
* Devuelve un ID único (o aleatorio) para el sistema.

Luego este módulo se reemplaza por el lector **RC522** u otro.

### 7. **storage.c** – Estructura de datos

Implementa el almacenamiento de cada registro:

* ID
* Índices de menú seleccionados
* Peso

Se puede usar:

* **Árbol binario** → acceso ordenado.
* **Hash table** → búsquedas muy rápidas.

Funciones:

* `storage_save()`
* `storage_find()`
* `storage_delete()`

Toda la memoria se mantiene en RAM; luego se exporta por serial.

### 8. **serial.c** – Comunicación con PC

Brinda dos posibilidades:

#### **1) UART clásica**

* Se conecta a un conversor USB–Serial (CH340/CP2102).
* `serial_init(baud)`
* `serial_sendRecord()`

La PC recibe datos por monitor serial.

#### **2) USB-CDC (nativo del LPC)**

* El dispositivo se monta como “puerto serie virtual”.
* El usuario ve un **COM/ttyUSB**.
* `serial_usb_init()` y funciones equivalentes.

La exportación consiste en enviar todos los registros uno por uno.

### 9. **utils.c** – Funciones auxiliares

Incluye:

* Conversiones numéricas sin `string.h` si fuese necesario.
* Funciones de formateo.
* Ayudas varias.

---

## main.c – Estructura general

* Inicialización de hardware.
* Inicialización de módulos.
* Configuración del tick del sistema.
* Bucle principal ejecutando la máquina de estados.

Ejemplo simplificado:

```
int main() {
    initHardware();
    ui_init();
    buttons_init();
    adc_init();
    dac_init();
    serial_init();

    changeState(STATE_BOOT);

    while(1) {
        buttons_update();
        stateMachine();
    }
}
```

---

## Máquina de estados

La arquitectura facilita:

* Actualizaciones ordenadas de pantalla.
* Desactivación de periféricos cuando no se usan.
* Evitar parpadeos y lecturas erráticas.
* Integrar RFID en el futuro sin reescribir el flujo principal.

---

## Notas finales

* La modularización favorece la portabilidad.
* La estructura es compatible con MCUXpresso.
* Cada módulo usa variables globales mínimas, definidas una vez y declaradas con `extern`.
