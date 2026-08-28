


/* Prototipos (implementa según HAL) */
void System_Init(void);
void Timer0_Init_for_sampling(uint32_t milis);
void ADC_Init_timer_trigger(void);
void DAC_Init(void);
void USB_CDC_Init_HW(void);
void DAC_SetVoltage(float volts);
int USB_CDC_Write(const char *s, int len);
int USB_CDC_Read(char *buf, int maxlen);

/* Simple media móvil */
float moving_average(uint16_t *buf, uint32_t idx, int N) {
    int i; uint32_t sum=0;
    int count = (N>BUFFER_SAMPLES)?BUFFER_SAMPLES:N;
    for(i=0;i<count;i++){
        uint32_t j = (idx + BUFFER_SAMPLES - i) % BUFFER_SAMPLES;
        sum += buf[j];
    }
    return (float)sum / count;
}

/* Procesa muestra: convierte a voltajes, calcula índice y actualiza DAC y USB */
void process_sample_and_telemetry(void) {
    uint32_t idx = (buf_write_idx==0)?0:( (buf_write_idx-1) % BUFFER_SAMPLES );
    float vt = (buf_temp[idx]/ADC_MAX_CODE) * VREF;
    float vh = (buf_hum[idx]/ADC_MAX_CODE) * VREF;
    float vl = (buf_light[idx]/ADC_MAX_CODE) * VREF;

    // Convertir a unidades (ej. TMP36: V->°C)
    float temp_c = (vt - 0.5f) * 100.0f; // TMP36 aproximado
    float hum_pct = (vh / VREF) * 100.0f; // si sensor linealizado
    float lux_rel = (vl / VREF) * 100.0f;

    // Índice de confort simple: (inverse temp deviation + humidity factor)
    float comfort = (25.0f - fabsf(25.0f - temp_c)) + (50.0f * (1.0f - fabsf(50.0f - hum_pct)/50.0f));
    // normalizar a 0..VREF
    float comfort_v = (comfort / 100.0f) * VREF;
    if (comfort_v < 0) comfort_v = 0;
    if (comfort_v > VREF) comfort_v = VREF;

    // Actualiza DAC
    DAC_SetVoltage(comfort_v);

    // Envia telemetría por USB (CSV)
    char line[128];
    int n = snprintf(line, sizeof(line), "T,%.2f,H,%.2f,L,%.2f,IDX,%.2f\r\n", temp_c, hum_pct, lux_rel, comfort);
    USB_CDC_Write(line, n);
}

/* Parser simple de comandos USB
   Comandos: "RATE x" (Hz), "START", "STOP", "EXPORT" (envía buffer CSV)
*/
void handle_usb_commands(void) {
    char buf[128];
    int len = USB_CDC_Read(buf, sizeof(buf)-1);
    if (len <= 0) return;
    buf[len] = 0;
    if (strncmp(buf,"RATE ",5)==0) {
        float r = atof(&buf[5]);
        if (r > 0.0f && r <= 10.0f) {
            sample_hz = r;
            Timer0_Init_for_sampling(sample_hz);
            USB_CDC_Write("OK RATE\r\n",9);
        }
    } else if (strncmp(buf,"STOP",4)==0) {
        running = false;
        USB_CDC_Write("OK STOP\r\n",9);
    } else if (strncmp(buf,"START",5)==0) {
        running = true;
        USB_CDC_Write("OK START\r\n",10);
    } else if (strncmp(buf,"EXPORT",6)==0) {
        // Export buffer as CSV (oldest->newest)
        uint32_t i;
        uint32_t start = (buf_write_idx < BUFFER_SAMPLES) ? 0 : (buf_write_idx % BUFFER_SAMPLES);
        uint32_t count = (buf_write_idx < BUFFER_SAMPLES) ? buf_write_idx : BUFFER_SAMPLES;
        for(i=0;i<count;i++){
            uint32_t idx = (start + i) % BUFFER_SAMPLES;
            float vt = (buf_temp[idx]/ADC_MAX_CODE) * VREF;
            float vh = (buf_hum[idx]/ADC_MAX_CODE) * VREF;
            float vl = (buf_light[idx]/ADC_MAX_CODE) * VREF;
            float temp_c = (vt - 0.5f) * 100.0f;
            float hum_pct = (vh / VREF) * 100.0f;
            float lux_rel = (vl / VREF) * 100.0f;
            char out[128];
            int m = snprintf(out, sizeof(out), "%.2f,%.2f,%.2f\r\n", temp_c, hum_pct, lux_rel);
            USB_CDC_Write(out, m);
        }
        USB_CDC_Write("OK EXPORT END\r\n",15);
    } else {
        USB_CDC_Write("ERR CMD\r\n",8);
    }
}

/* main */
int main(void) {
    System_Init();
    DAC_Init();
    USB_CDC_Init_HW();
    ADC_Init_timer_trigger();
    Timer0_Init_for_sampling(sample_hz);

    // Enable interrupts globally
    while (1) {
        if (new_sample_flag && running) {
            new_sample_flag = false;
            process_sample_and_telemetry();
        }
        handle_usb_commands();
        // baja prioridad: dormir o WFI
    }
}

/* === Implementar / adaptar las funciones ===
   - System_Init(): habilita clocks, pines, NVIC.
   - Timer0_Init_for_sampling(freq): configura MR0 y habilita salida de match que arranca ADC.
   - ADC_Init_timer_trigger(): configura ADC para start on MAT0.1 y las 3 canales.
   - ADC_ReadChannel_raw(ch): lee registro ADC global o datos en buffer.
   - DAC_Init() / DAC_SetVoltage(volts): inicializa DAC y escribe valor.
   - USB_CDC_*: inicializa stack USB CDC y provee read/write blocking o no-blocking.
*/

void drawMenu(uint16_t peso) {
    for(uint8_t i = 0; i<4 ; i++){
        for (int i = 0; i < 3; i++) {
            lcd_setCursor(0, i);
            if (i == cursorIndex) lcd_print(">");
            else lcd_print(" ");
            lcd_print(itemNames[i]);
            lcd_print(itemOptions[i][itemSelection[i]]);
        }
        lcd_setCursor(0, 3);
        sprintf(buffer, "Peso: %d kg", peso);
        lcd_print("Peso  ");
        lcd_print(kilos_str);
        lcd_print(" kg");
    }
}

void drawMenu(uint16_t peso) {
    for(uint8_t i = 0; i<4 ; i++){
        for (int i = 0; i < 3; i++) {
            menu[i][0] = "                    ";
            if (i == cursorIndex) menu[i][0] = ">";
            else menu[i][0] = " ";
            menu[i][1] = itemNames[i];
            menu[i][7] = itemOptions[i][itemSelection[i]];
        }
        menu[3][1] = itemNames[i];
        menu[3][7] = kilos_str;
        menu[3][10] =" kg";
    }
}

void drawMenu(uint16_t peso) {
    int i, j;
    // Líneas de menú configurables
    for (i = 0; i < 3; i++) {
        // Rellenar con espacios
        for (j = 0; j < sizeof(menu[i]) - 1; j++) {
            menu[i][j] = ' ';
        }
        menu[i][sizeof(menu[i]) - 1] = '\0'; // Null-terminate

        // Cursor
        menu[i][0] = (i == cursorIndex) ? '>' : ' ';

        // Nombre de la opción
        for (j = 0; j < xOffSet && itemNames[i][j] != '\0'; j++) {
            menu[i][1 + j] = itemNames[i][j];
        }

        // Valor seleccionado
        const char* opt = itemOptions[i][itemSelection[i]];
        for (j = 0; j < sizeof(menu[i]) - 8 && opt[j] != '\0'; j++) {
            menu[i][7 + j] = opt[j];
        }
    }

    // Línea de peso
    for (j = 0; j < sizeof(menu[3]) - 1; j++) {
        menu[3][j] = ' ';
    }
    menu[3][sizeof(menu[3]) - 1] = '\0';

    // Nombre "Peso"
    for (j = 0; j < xOffSet && itemNames[3][j] != '\0'; j++) {
        menu[3][1 + j] = itemNames[3][j];
    }

    // Valor de kilos
    for (j = 0; j < 3 && kilos_str[j] != '\0'; j++) {
        menu[3][7 + j] = kilos_str[j];
    }

    // Sufijo " kg"
    menu[3][10] = ' ';
    menu[3][11] = 'k';
    menu[3][12] = 'g';
}


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// ---- Estructura de datos ----
typedef struct {
    uint32_t id;       // ID leído del RFID
    uint8_t tipo;      // índice de tipo seleccionado
    uint8_t estado;    // índice de estado
    uint8_t categoria; // índice de categoría
    int pesoKg;        // peso cargado
} Registro;

// ---- Nodo del árbol binario ----
typedef struct Nodo {
    Registro data;
    struct Nodo* izq;
    struct Nodo* der;
} Nodo;

// ---- Crear un nuevo nodo ----
Nodo* crearNodo(Registro r) {
    Nodo* nuevo = (Nodo*) malloc(sizeof(Nodo));
    if (!nuevo) return NULL;
    nuevo->data = r;
    nuevo->izq = NULL;
    nuevo->der = NULL;
    return nuevo;
}

// ---- Insertar nuevo registro en el árbol ----
// Si el ID ya existe, actualiza los datos.
Nodo* insertarNodo(Nodo* raiz, Registro r) {
    if (raiz == NULL) return crearNodo(r);

    if (r.id < raiz->data.id)
        raiz->izq = insertarNodo(raiz->izq, r);
    else if (r.id > raiz->data.id)
        raiz->der = insertarNodo(raiz->der, r);
    else
        raiz->data = r;  // Si ya existe, se actualizan los datos

    return raiz;
}

// ---- Buscar registro por ID ----
Nodo* buscarNodo(Nodo* raiz, uint32_t id) {
    if (raiz == NULL) return NULL;
    if (id == raiz->data.id) return raiz;
    if (id < raiz->data.id) return buscarNodo(raiz->izq, id);
    return buscarNodo(raiz->der, id);
}

// ---- Recorrer árbol (por ejemplo, para exportar por USB) ----
void recorrerInOrden(Nodo* raiz) {
    if (raiz == NULL) return;
    recorrerInOrden(raiz->izq);
    printf("ID: %lu | Tipo: %d | Estado: %d | Cat: %d | Peso: %d kg\n",
           raiz->data.id, raiz->data.tipo, raiz->data.estado,
           raiz->data.categoria, raiz->data.pesoKg);
    recorrerInOrden(raiz->der);
}

// suponiendo display 20x4, menu[][20] y oldLines[][20] declarados y kilos_str ya listo

// Inicializar oldLines con cadenas vacías para forzar primera actualización
void initOldLines(void) {
    for (uint8_t r = 0; r < 4; r++) {
        for (uint8_t c = 0; c < 19; c++) oldLines[r][c] = ' ';
        oldLines[r][19] = '\0';
    }
}

// Compara text contra oldLines[line] sin usar strcmp
static uint8_t lineChanged(uint8_t line, const char *text) {
    for (uint8_t i = 0; i < 19; i++) {
        char a = oldLines[line][i];
        char b = text[i];
        if (b == '\0') { // resto debe ser espacios
            // si alguno de los restantes en oldLines no es espacio, hay cambio
            for (uint8_t k = i; k < 19; k++) if (oldLines[line][k] != ' ') return 1;
            return 0; // iguales
        }
        if (a != b) return 1;
    }
    return 0;
}

// Copia text en oldLines[line], rellenando con espacios y colocando '\0'
static void copyToOldLine(uint8_t line, const char *text) {
    uint8_t i = 0;
    for (; i < 19; i++) {
        if (text[i] == '\0') break;
        oldLines[line][i] = text[i];
    }
    // rellena con espacios hasta 19 chars
    for (; i < 19; i++) oldLines[line][i] = ' ';
    oldLines[line][19] = '\0';
}

// Actualiza una línea concreta en el LCD sólo si cambió
void updateLine(uint8_t line, const char *text) {
    if (!lineChanged(line, text)) return;  // nada que hacer

    // actualizamos el buffer viejo
    copyToOldLine(line, text);

    // escribir directamente en la fila: moved cursor y print de 19 caracteres
    lcd_setCursor(0, line);         // asegurate que la función colocca cursor correctamente
    lcd_print(oldLines[line]);      // tu lcd_print debería aceptar '\0' terminated string
}

// Actualiza las 4 líneas (llamar tras actualizar menu[][] con drawMenu)
void updateDisplay(void) {
    updateLine(0, menu[0]);
    updateLine(1, menu[1]);
    updateLine(2, menu[2]);
    updateLine(3, menu[3]);
}

void initOldLines(void)
static uint8_t lineChanged(uint8_t line, const char *text)
static void copyToOldLine(uint8_t line, const char *text)
void updateLine(uint8_t line, const char *text)
void updateDisplay(void)