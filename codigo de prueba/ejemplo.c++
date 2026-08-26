#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Mapeo de pines a GND
#define BTN_RIGHT  13
#define BTN_LEFT   26
#define BTN_UP     25
#define BTN_DOWN   27
#define BTN_SELECT 32

const int totalPreguntas = 30;
int respuestaCorrecta[totalPreguntas];
int respuestaAlumno[totalPreguntas]; // -1 = vacia / sin responder

int preguntaActual = 0;
bool pantallaConfirmacion = false;
bool examenTerminado = false;

unsigned long ultimoBoton = 0;
const int debounceDelay = 200;

char letras[4] = {'A', 'B', 'C', 'D'};

void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  delay(100);
  lcd.init();
  lcd.backlight();

  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  for (int i = 0; i < totalPreguntas; i++) {
    respuestaAlumno[i] = -1;  // Todas arrancan vacias
    respuestaCorrecta[i] = 0; // Solucionario de prueba ('A')
  }

  mostrarPregunta();
}

void loop() {
  if (examenTerminado) return;
  if (millis() - ultimoBoton < debounceDelay) return;

  if (pantallaConfirmacion) {
    manejarConfirmacion();
    return;
  }

  if (digitalRead(BTN_RIGHT) == LOW) {
    moverOpcion(1);
  }
  else if (digitalRead(BTN_LEFT) == LOW) {
    moverOpcion(-1);
  }
  else if (digitalRead(BTN_UP) == LOW) {
    moverPregunta(-1);
  }
  else if (digitalRead(BTN_DOWN) == LOW) {
    moverPregunta(1);
  }
  else if (digitalRead(BTN_SELECT) == LOW) {
    pantallaConfirmacion = true;
    mostrarConfirmacion();
    ultimoBoton = millis();
  }
}

void moverOpcion(int dir) {
  int actual = respuestaAlumno[preguntaActual];
  
  // Si estaba vacia (-1), la primera pulsacion selecciona 'A' (0) o 'D' (3)
  if (actual == -1) {
    actual = (dir == 1) ? 0 : 3;
  } else {
    actual = (actual + dir + 4) % 4;
  }
  
  respuestaAlumno[preguntaActual] = actual;
  mostrarPregunta();
  ultimoBoton = millis();
}

void moverPregunta(int dir) {
  preguntaActual += dir;
  if (preguntaActual < 0) preguntaActual = 0;
  if (preguntaActual >= totalPreguntas) preguntaActual = totalPreguntas - 1;
  mostrarPregunta();
  ultimoBoton = millis();
}

void mostrarPregunta() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pregunta: ");
  lcd.print(preguntaActual + 1);
  lcd.print("/");
  lcd.print(totalPreguntas);

  lcd.setCursor(0, 1);
  int sel = respuestaAlumno[preguntaActual];
  
  for (int i = 0; i < 4; i++) {
    if (i == sel) { // Solo dibuja corchetes si fue respondida (sel != -1)
      lcd.print('[');
      lcd.print(letras[i]);
      lcd.print(']');
    } else {
      lcd.print(' ');
      lcd.print(letras[i]);
      lcd.print(' ');
    }
  }
}

void mostrarConfirmacion() {
  int sinResponder = 0;
  for (int i = 0; i < totalPreguntas; i++) {
    if (respuestaAlumno[i] == -1) sinResponder++;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  if (sinResponder > 0) {
    lcd.print("Faltan ");
    lcd.print(sinResponder);
    lcd.print(" sin resp");
  } else {
    lcd.print("Confirmar envio?");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("SEL=Si  IZQ=No");
}

void manejarConfirmacion() {
  if (digitalRead(BTN_SELECT) == LOW) {
    finalizarExamen();
    ultimoBoton = millis();
  }
  else if (digitalRead(BTN_LEFT) == LOW) {
    pantallaConfirmacion = false;
    mostrarPregunta();
    ultimoBoton = millis();
  }
}

void finalizarExamen() {
  examenTerminado = true;
  int puntaje = 0;
  for (int i = 0; i < totalPreguntas; i++) {
    // Solo suma si respondio y la respuesta es correcta
    if (respuestaAlumno[i] != -1 && respuestaAlumno[i] == respuestaCorrecta[i]) {
      puntaje++;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Examen terminado");
  lcd.setCursor(0, 1);
  lcd.print("Puntaje: ");
  lcd.print(puntaje);
  lcd.print("/");
  lcd.print(totalPreguntas);

  Serial.print("RESULTADO,");
  Serial.print(puntaje);
  Serial.print(",");
  Serial.println(totalPreguntas);
}
