# LiquidCrystal_I2C_LPC  
Controlador para pantallas LCD HD44780 mediante expansor I2C PCF8574 (LPC1769)

---

## 📘 Descripción
Esta librería permite controlar pantallas LCD estándar (16x2, 20x4, etc.) con controlador **HD44780** usando un expansor **PCF8574** conectado por bus **I2C**.  
Está escrita para microcontroladores **NXP LPC17xx** y usa las funciones del **driver I2C del SDK (chip.h)**.

---

## ⚙️ Conexiones por defecto del módulo PCF8574

| Pin PCF8574 | Señal LCD | Descripción                  |
|--------------|------------|------------------------------|
| P0           | RS         | Register Select              |
| P1           | RW         | Read/Write (normalmente GND) |
| P2           | EN         | Enable                       |
| P3           | BL         | Backlight control            |
| P4           | D4         | Data bit 4                   |
| P5           | D5         | Data bit 5                   |
| P6           | D6         | Data bit 6                   |
| P7           | D7         | Data bit 7                   |

> Esta es la configuración **más común** en los módulos I2C tipo PCF8574T (dirección 0x27).  
> Si el pinout difiere, se puede adaptar el código ajustando la máscara de bits usada en `lcd_sendNibble()`.

---

## 🧩 Requisitos

- Bus I2C inicializado previamente mediante `Chip_I2C_Init()` y `I2C_MasterTransferData()`.  
- Alimentación del módulo: **VCC = 5V**, **GND = GND común con el microcontrolador**.  
- Librería depende de:  
  ```c
  #include "LiquidCrystal_I2C_LPC.h"
  #include "LPC17xx.h"
  #include "lpc17xx_i2c.h"
