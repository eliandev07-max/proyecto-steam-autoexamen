#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Dirección I2C típica: 0x27 (si no enciende, prueba cambiar a 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
  // Inicializar pantalla LCD
  lcd.init();
  lcd.backlight();

  // Mensaje en la primera fila (0, 0)
  lcd.setCursor(0, 0);
  lcd.print("Prueba LCD OK!");

  // Mensaje en la segunda fila (0, 1)
  lcd.setCursor(0, 1);
  lcd.print("ESP32 Funcionando");
}

void loop() {
  // Sin código en el loop
}
