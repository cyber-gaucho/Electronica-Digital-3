


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

