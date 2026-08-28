// ESP32-S3 DevKitC-1 + OLED SSD1306 (Adafruit) - Menú con 3 ítems configurables + 1 lectura analógica (celda de carga simulada).
// Pines adaptados para ESP32-S3-DevKitC-1.
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------- CONFIGURACIÓN DE PINES ----------
#define SDA_PIN 8
#define SCL_PIN 9
#define I2C_ADDR 0x27

#define BTN_UP_PIN     4
#define BTN_DOWN_PIN   5
#define BTN_LEFT_PIN   6
#define BTN_RIGHT_PIN  7
#define BTN_SELECT_PIN 15

#define LOAD_CELL_PIN  11  // Entrada analógica (potenciómetro)

// -------- PARÁMETROS ----------
#define SERIAL_BAUD 115200
#define SCREEN_WIDTH 20
#define SCREEN_HEIGHT 4
#define DEBOUNCE_MS 25
#define BLINK_INTERVAL_MS 400
#define UI_REFRESH_MS 100

#define BUFFER_SIZE 32

LiquidCrystal_I2C lcd(I2C_ADDR, SCREEN_WIDTH, SCREEN_HEIGHT); // dirección, columnas, filas

// -------- MENU / OPCIONES ----------
const char headerID[] = "0123456789ABCDE";

const char* itemNames[4] = {
  "Sexo ",
  "Color",
  "Categ",
  "Peso "
};
const uint8_t xOffSet = 6;  // Ancho de la categoria mas larga

const char* options0[] = {"-", "Macho", "Hembra"};
const char* options1[] = {"Negro", "Colorado", "Careta", "Pampa"};
const char* options2[] = {"Ternero", "Vaquillona", "Novillo", "Toro", "Vaca", "Vaca Prenada"};

const char** itemOptions[3] = {options0, options1, options2};
const uint8_t itemOptionsCount[3] = {
  sizeof(options0) / sizeof(options0[0]),
  sizeof(options1) / sizeof(options1[0]),
  sizeof(options2) / sizeof(options2[0])
};

uint8_t itemSelection[3] = {0, 0, 0};
int cursorIndex = 0;  // 0..3 (el 3 es lectura, no configurable)

struct Button {
  uint8_t pin;
  bool stableState;
  bool lastRead;
  unsigned long lastDebounceTime;
  bool pressedEvent;
};

Button btnUp = {BTN_UP_PIN, HIGH, HIGH, 0, false};
Button btnDown = {BTN_DOWN_PIN, HIGH, HIGH, 0, false};
Button btnLeft = {BTN_LEFT_PIN, HIGH, HIGH, 0, false};
Button btnRight = {BTN_RIGHT_PIN, HIGH, HIGH, 0, false};
Button btnSelect = {BTN_SELECT_PIN, HIGH, HIGH, 0, false};

unsigned long lastBlinkToggle = 0;
bool blinkState = true;
unsigned long lastUIRefresh = 0;

int adcBuffer[BUFFER_SIZE] = {0};
int adcInd = 0;
long adcSum = 0;
int adcAvg = 0;

// -------- FUNCIONES DE BOTONES ----------
void setupButtons() {
  pinMode(btnUp.pin, INPUT_PULLUP);
  pinMode(btnDown.pin, INPUT_PULLUP);
  pinMode(btnLeft.pin, INPUT_PULLUP);
  pinMode(btnRight.pin, INPUT_PULLUP);
  pinMode(btnSelect.pin, INPUT_PULLUP);
}

void updateButton(Button &b) {
  bool reading = digitalRead(b.pin);
  if (reading != b.lastRead) {
    b.lastDebounceTime = millis();
    b.lastRead = reading;
  }
  if ((millis() - b.lastDebounceTime) > DEBOUNCE_MS) {
    if (reading != b.stableState) {
      b.stableState = reading;
      if (b.stableState == LOW) b.pressedEvent = true;
    }
  }
}

bool consumePressed(Button &b) {
  if (b.pressedEvent) {
    b.pressedEvent = false;
    return true;
  }
  return false;
}

// -------- INTERFAZ --------
// void drawHeader() {
//   // display.setTextSize(1);
//   // display.setTextColor(SSD1306_WHITE);
//   // display.setCursor(0, 0);
//   // display.print(headerID);

//   // previamente hacer lcd.clear()

//   lcd.setCursor(0,1);
//   lcd.print(headerID);

//   // posteriormente hacer un breve delay para que se vea el ID

// }
String menu[4];
void drawMenu(int cargaKg) {
  for (int i=0; i<4; i++) {
    // Imprimir categorias
    if (i == cursorIndex) menu[i] = ">";  // Si es la seleccionada imprime lo indica con >
    else                  menu[i] = " ";

    menu[i] = menu[i] + itemNames[i] + " ";    // Imprime el nombre de la categoria

    // Imprimir contenidos o peso
    if (i < 3) {
      menu[i] = menu[i] + itemOptions[i][itemSelection[i]];  // Imprime el contenido de la fila
    } else {
      menu[i] = menu[i] + cargaKg + " kg";
    }
  }
}
// void drawMenu(int cargaKg) {
//   for (int i=0; i<3; i++) {
//     // Imprimir categorias
//     lcd.setCursor(0, i);                    // Posisiona cursor en la fila

//     if (i == cursorIndex) lcd.print(">");  // Si es la seleccionada imprime lo indica con >
//     else lcd.print(" "); 

//     lcd.print(itemNames[i]);                // Imprime el nombre de la categoria
//     lcd.print(" ");
    
//     lcd.setCursor(xOffSet + 1, i);          // Mueve el cursor al offset para el contenido (para que quede parejo)

//     // Imprimir contenidos o peso
//     if (i < 3) {
//       lcd.print(itemOptions[i][itemSelection[i]]);  // Imprime el contenido de la fila
//     } else {
//       lcd.print(cargaKg);
//       lcd.print(" kg");
//     }
//   }
// }

String oldLines[4];  // Guarda lo último mostrado

void updateLine(int line, const String &text) {
  if (oldLines[line] != text) {           // Solo actualiza si hay cambio
    lcd.setCursor(0, line);
    lcd.print("                   ");    // Borra solo esa línea (20 espacios)
    lcd.setCursor(0, line);
    lcd.print(text);
    oldLines[line] = text;
  }
}

void updateDisplay() { 
  updateLine(0, menu[0]);
  updateLine(1, menu[1]);
  updateLine(2, menu[2]);
  updateLine(3, menu[3]);
}

void forceUpdate(){
  lcd.clear();
  for (int line=0; line<3; line++) {
    lcd.setCursor(0, line);
    lcd.print(oldLines[line]);
  }
}

void exportCSV(int cargaKg) {
  String csv = "";
  for (int i = 0; i < 3; ++i) {
    if (i) csv += ",";
    csv += String(itemOptions[i][itemSelection[i]]);
  }
  csv += ",";
  csv += String(cargaKg);
  Serial.println(csv);

  lcd.clear();
  lcd.setCursor(3,1);
  lcd.print("Exportando...");

  delay(700);
  forceUpdate();
}

// -------- ADC --------
void updateAdcAvg(int adcRaw) {
  adcSum -= adcBuffer[adcInd];          // restar el valor que reemplazamos
  
  adcBuffer[adcInd] = adcRaw;           // escribir nuevo dato
  
  adcSum += adcRaw;                     // sumar el nuevo dato
 
  adcInd = (adcInd + 1) % BUFFER_SIZE;  // avanzar índice
  
  adcAvg = adcSum / BUFFER_SIZE;        // calcular promedio
}

// -------- SETUP Y LOOP --------
void setup() {
  Serial.begin(SERIAL_BAUD);
  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  setupButtons();
  pinMode(LOAD_CELL_PIN, INPUT);  // Entrada analógica

  // Acá va la pantalla de inicio
  lcd.setCursor(0, 1);
  lcd.print("Sistema Ganadero");
  lcd.setCursor(0, 2);
  lcd.print("Iniciando...");

  delay(1000);
}

void loop() {
  unsigned long now = millis();

  // Leer ADC y convertir a kg
  int adcRaw = analogRead(LOAD_CELL_PIN);
  updateAdcAvg(adcRaw);
  int cargaKg = map(adcAvg, 0, 4095, 0, 1000);

  // Actualizar botones
  updateButton(btnUp);
  updateButton(btnDown);
  updateButton(btnLeft);
  updateButton(btnRight);
  updateButton(btnSelect);

  // Eventos
  if (consumePressed(btnUp)) {
    cursorIndex--;
    if (cursorIndex < 0) cursorIndex = 3;
  }
  if (consumePressed(btnDown)) {
    cursorIndex++;
    if (cursorIndex > 3) cursorIndex = 0;
  }
  if (cursorIndex < 3) {  // solo si está en ítems configurables
    if (consumePressed(btnLeft)) {
      int i = cursorIndex;
      if (itemSelection[i] == 0) itemSelection[i] = itemOptionsCount[i] - 1;
      else itemSelection[i]--;
    }
    if (consumePressed(btnRight)) {
      int i = cursorIndex;
      itemSelection[i]++;
      if (itemSelection[i] >= itemOptionsCount[i]) itemSelection[i] = 0;
    }
  }
  if (consumePressed(btnSelect)) {
    exportCSV(cargaKg);
  }

  // Parpadeo cursor
  if (now - lastBlinkToggle >= BLINK_INTERVAL_MS) {
    blinkState = !blinkState;
    lastBlinkToggle = now;
  }

  // Refresco pantalla
  if (now - lastUIRefresh >= UI_REFRESH_MS) {
    lastUIRefresh = now;
    
    // drawHeader();    // Este lo vamos a hacer en la SPI_IRQ por un segundo
    drawMenu(cargaKg);
    updateDisplay();
  }

  delay(5);
}
