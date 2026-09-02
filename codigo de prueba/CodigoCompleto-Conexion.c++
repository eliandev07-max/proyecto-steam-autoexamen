#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- Config Access Point ---
const char* ssid = "Autoexamen";
const char* password = "examen123";
WebServer server(80);
DNSServer dnsServer;

// Mapeo de pines
#define BTN_RIGHT  13
#define BTN_LEFT   26
#define BTN_UP     25
#define BTN_DOWN   27
#define BTN_SELECT 32

const int totalPreguntas = 30;
int respuestaCorrecta[totalPreguntas];
int respuestaAlumno[totalPreguntas]; 

// Lista de alumnos disponibles
const int totalListaAlumnos = 6;
String listaAlumnos[totalListaAlumnos] = {
  "Agustin",
  "Maria",
  "Juan",
  "Lucas",
  "Sofia",
  "Mateo"
};

int indiceAlumnoSeleccionado = 0;
String alumnoActual = "";
bool alumnoConfirmado = false; // True cuando el alumno eligió su nombre en el LCD

int preguntaActual = 0;
bool pantallaConfirmacion = false;
bool examenTerminado = false;
bool examenIniciado = false; 

unsigned long ultimoBoton = 0;
const int debounceDelay = 200;

char letras[4] = {'A', 'B', 'C', 'D'};

// Estructura para almacenar notas de la sesión en RAM
struct RegistroNota {
  String nombre;
  int puntaje;
  bool realizado;
};

RegistroNota historialNotas[totalListaAlumnos];

// ================= DECLARACIÓN DE PROTOTIPOS =================
void mostrarSeleccionAlumno();
void mostrarEsperandoProfe();
void mostrarPregunta();
void mostrarConfirmacion();
void manejarConfirmacion();
void moverOpcion(int dir);
void moverPregunta(int dir);
void finalizarExamen();

// ================= VISTAS Y FUNCIONES LCD =================

void mostrarSeleccionAlumno() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Elegir alumno:");
  lcd.setCursor(0, 1);
  lcd.print(">");
  lcd.print(listaAlumnos[indiceAlumnoSeleccionado]);
}

void mostrarEsperandoProfe() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hola ");
  lcd.print(alumnoActual);
  lcd.setCursor(0, 1);
  lcd.print("Esperando profe..");
}

void mostrarPregunta() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("P");
  lcd.print(preguntaActual + 1);
  lcd.print("/");
  lcd.print(totalPreguntas);
  lcd.print(" ");
  lcd.print(alumnoActual.substring(0, 8)); // Nombre corto arriba

  lcd.setCursor(0, 1);
  int sel = respuestaAlumno[preguntaActual];

  for (int i = 0; i < 4; i++) {
    if (i == sel) {
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

void moverOpcion(int dir) {
  int actual = respuestaAlumno[preguntaActual];

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

void finalizarExamen() {
  examenTerminado = true;
  int puntaje = 0;
  for (int i = 0; i < totalPreguntas; i++) {
    if (respuestaAlumno[i] != -1 && respuestaAlumno[i] == respuestaCorrecta[i]) {
      puntaje++;
    }
  }

  // Guardar en el historial de RAM
  historialNotas[indiceAlumnoSeleccionado].puntaje = puntaje;
  historialNotas[indiceAlumnoSeleccionado].realizado = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Examen terminado");
  lcd.setCursor(0, 1);
  lcd.print("Puntaje: ");
  lcd.print(puntaje);
  lcd.print("/");
  lcd.print(totalPreguntas);
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

// ================= WEB DEL PROFESOR =================

String paginaPrincipal() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Autoexamen - Panel del Profesor</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; max-width: 650px; margin: 20px auto; padding: 0 15px; background-color: #f4f7f6; color: #333; }
    h1 { font-size: 22px; text-align: center; color: #2c3e50; }
    .card { background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); margin-bottom: 20px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(130px, 1fr)); gap: 10px; margin-top: 15px; }
    .item { background: #fafafa; border: 1px solid #ddd; padding: 8px; border-radius: 5px; text-align: center; }
    .item span { font-weight: bold; display: block; margin-bottom: 5px; font-size: 14px; }
    .options { display: flex; justify-content: space-around; }
    .options label { font-size: 13px; cursor: pointer; }
    button { padding: 12px 20px; margin-top: 15px; cursor: pointer; font-size: 16px; border-radius: 5px; font-weight: bold; border: none; width: 100%; }
    .guardar { background: #3498db; color: white; }
    .iniciar { background: #27ae60; color: white; }
    .status-box { font-size: 18px; text-align: center; font-weight: bold; padding: 12px; border-radius: 5px; }
    .esperando { background: #ffeaa7; color: #d35400; }
    .listo { background: #74b9ff; color: #0984e3; }
    .finalizado { background: #55efc4; color: #00897b; }
    .respuesta-tag { display: inline-block; padding: 2px 6px; border-radius: 3px; font-size: 12px; margin: 2px; }
    .correcta { background: #2ecc71; color: white; }
    .incorrecta { background: #e74c3c; color: white; }
    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { border: 1px solid #ddd; padding: 8px; text-align: center; }
    th { background-color: #f2f2f2; }
  </style>
</head>
<body>
  <h1>Panel de Control del Examen</h1>
  
  <div class="card">
    <h2>1. Clave de Respuestas Correctas</h2>
    <form action="/guardar" method="POST">
      <div class="grid">
)rawliteral";

  for (int i = 0; i < totalPreguntas; i++) {
    html += "<div class='item'><span>P" + String(i + 1) + "</span><div class='options'>";
    for (int j = 0; j < 4; j++) {
      html += "<label><input type='radio' name='resp" + String(i) + "' value='" + String(j) + "'";
      if (respuestaCorrecta[i] == j) html += " checked";
      html += ">" + String(letras[j]) + "</label>";
    }
    html += "</div></div>";
  }

  html += R"rawliteral(
      </div>
      <button type="submit" class="guardar">Guardar Solucionario</button>
    </form>
  </div>

  <div class="card">
    <h2>2. Control del Examen</h2>
    <div id="estado-contenedor" class="status-box esperando">Cargando estado...</div>
    <form action="/iniciar" method="POST" id="form-iniciar">
      <button type="submit" id="btn-iniciar" class="iniciar" disabled>Iniciar Examen para el Alumno</button>
    </form>
  </div>

  <div class="card">
    <h2>3. Desglose del Examen Actual</h2>
    <div id="desglose">Sin datos de examen activo.</div>
  </div>

  <div class="card">
    <h2>4. Resumen de Notas de la Sesion</h2>
    <div id="tabla-historial">Cargando notas...</div>
  </div>

  <script>
    function consultarEstado() {
      fetch('/api_estado')
        .then(response => response.json())
        .then(data => {
          const contenedor = document.getElementById('estado-contenedor');
          const btnIniciar = document.getElementById('btn-iniciar');
          const desglose = document.getElementById('desglose');
          const tablaHistorial = document.getElementById('tabla-historial');

          // 1. Estado en Vivo
          if (!data.confirmado) {
            contenedor.className = "status-box esperando";
            contenedor.innerHTML = "Esperando que un alumno seleccione su nombre en el LCD...";
            btnIniciar.disabled = true;
            btnIniciar.innerText = "Esperando alumno...";
            desglose.innerHTML = "Ningún alumno seleccionado.";
          } else if (!data.iniciado) {
            contenedor.className = "status-box listo";
            contenedor.innerHTML = "Alumno preparado: <b>" + data.alumno + "</b>";
            btnIniciar.disabled = false;
            btnIniciar.innerText = "Habilitar Examen a " + data.alumno;
            desglose.innerHTML = "Esperando que el profesor inicie el examen.";
          } else if (!data.terminado) {
            contenedor.className = "status-box esperando";
            contenedor.innerHTML = "En progreso: <b>" + data.alumno + "</b> resolviendo en LCD...";
            btnIniciar.disabled = true;
            btnIniciar.innerText = "Examen en Curso";
            desglose.innerHTML = "El alumno está contestando las preguntas...";
          } else {
            contenedor.className = "status-box finalizado";
            contenedor.innerHTML = "¡EXAMEN RECIBIDO!<br>Alumno: <b>" + data.alumno + "</b><br>Puntaje: " + data.puntaje + " / " + data.total;
            btnIniciar.disabled = false;
            btnIniciar.innerText = "Reiniciar / Siguiente Alumno";

            let htmlDesglose = "<div style='display:grid; grid-template-columns: repeat(auto-fill, minmax(80px, 1fr)); gap:5px;'>";
            data.respuestas.forEach((resp, idx) => {
              let clase = (resp.alumno === resp.correcta) ? "correcta" : "incorrecta";
              htmlDesglose += `<div style='border:1px solid #ccc; padding:4px; text-align:center; border-radius:4px;'>
                <b>P${idx + 1}:</b> <span class='respuesta-tag ${clase}'>${resp.alumno}</span>
              </div>`;
            });
            htmlDesglose += "</div>";
            desglose.innerHTML = htmlDesglose;
          }

          // 2. Tabla de Historial
          let tablaHtml = "<table><tr><th>Alumno</th><th>Estado</th><th>Nota</th></tr>";
          data.historial.forEach(h => {
            tablaHtml += `<tr>
              <td>${h.nombre}</td>
              <td>${h.realizado ? 'Completado' : 'Pendiente'}</td>
              <td><b>${h.realizado ? h.puntaje + ' / ' + data.total : '-'}</b></td>
            </tr>`;
          });
          tablaHtml += "</table>";
          tablaHistorial.innerHTML = tablaHtml;
        });
    }

    setInterval(consultarEstado, 2000);
    consultarEstado();
  </script>
</body>
</html>
)rawliteral";

  return html;
}

void handleRoot() {
  server.send(200, "text/html", paginaPrincipal());
}

void handleGuardar() {
  for (int i = 0; i < totalPreguntas; i++) {
    String campo = "resp" + String(i);
    if (server.hasArg(campo)) {
      respuestaCorrecta[i] = server.arg(campo).toInt();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleIniciar() {
  for (int i = 0; i < totalPreguntas; i++) {
    respuestaAlumno[i] = -1;
  }
  examenIniciado = true;
  examenTerminado = false;
  pantallaConfirmacion = false;
  preguntaActual = 0;
  
  mostrarPregunta(); 
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleApiEstado() {
  String json = "{";
  json += "\"confirmado\":" + String(alumnoConfirmado ? "true" : "false") + ",";
  json += "\"iniciado\":" + String(examenIniciado ? "true" : "false") + ",";
  json += "\"terminado\":" + String(examenTerminado ? "true" : "false") + ",";
  json += "\"alumno\":\"" + alumnoActual + "\",";
  
  int puntaje = 0;
  String respuestasArr = "[";
  for (int i = 0; i < totalPreguntas; i++) {
    int resp = respuestaAlumno[i];
    if (resp != -1 && resp == respuestaCorrecta[i]) puntaje++;
    
    respuestasArr += "{\"alumno\":\"";
    respuestasArr += (resp == -1) ? "-" : String(letras[resp]);
    respuestasArr += "\",\"correcta\":\"" + String(letras[respuestaCorrecta[i]]) + "\"}";
    if (i < totalPreguntas - 1) respuestasArr += ",";
  }
  respuestasArr += "]";
  
  json += "\"puntaje\":" + String(puntaje) + ",";
  json += "\"total\":" + String(totalPreguntas) + ",";
  json += "\"respuestas\":" + respuestasArr + ",";

  // Enviar lista de notas acumuladas
  String histArr = "[";
  for (int i = 0; i < totalListaAlumnos; i++) {
    histArr += "{\"nombre\":\"" + historialNotas[i].nombre + "\",";
    histArr += "\"realizado\":" + String(historialNotas[i].realizado ? "true" : "false") + ",";
    histArr += "\"puntaje\":" + String(historialNotas[i].puntaje) + "}";
    if (i < totalListaAlumnos - 1) histArr += ",";
  }
  histArr += "]";
  json += "\"historial\":" + histArr;

  json += "}";

  server.send(200, "application/json", json);
}

// ================= SETUP & LOOP =================

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

  // Inicializar historial
  for (int i = 0; i < totalListaAlumnos; i++) {
    historialNotas[i].nombre = listaAlumnos[i];
    historialNotas[i].puntaje = 0;
    historialNotas[i].realizado = false;
  }

  for (int i = 0; i < totalPreguntas; i++) {
    respuestaAlumno[i] = -1;   
    respuestaCorrecta[i] = 0;  
  }

  WiFi.softAP(ssid, password);
  dnsServer.start(53, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/guardar", HTTP_POST, handleGuardar);
  server.on("/iniciar", HTTP_POST, handleIniciar);
  server.on("/api_estado", HTTP_GET, handleApiEstado);
  
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/");
    server.send(302, "text/plain", "");
  });

  server.begin();
  mostrarSeleccionAlumno();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient(); 

  if (millis() - ultimoBoton < debounceDelay) return;

  // FASE 1: Seleccionar Alumno en el LCD
  if (!alumnoConfirmado) {
    if (digitalRead(BTN_DOWN) == LOW) {
      indiceAlumnoSeleccionado = (indiceAlumnoSeleccionado + 1) % totalListaAlumnos;
      mostrarSeleccionAlumno();
      ultimoBoton = millis();
    } 
    else if (digitalRead(BTN_UP) == LOW) {
      indiceAlumnoSeleccionado = (indiceAlumnoSeleccionado - 1 + totalListaAlumnos) % totalListaAlumnos;
      mostrarSeleccionAlumno();
      ultimoBoton = millis();
    } 
    else if (digitalRead(BTN_SELECT) == LOW) {
      alumnoActual = listaAlumnos[indiceAlumnoSeleccionado];
      alumnoConfirmado = true;
      mostrarEsperandoProfe();
      ultimoBoton = millis();
    }
    return;
  }

  // FASE 2: Esperando habilitación del Profesor
  if (!examenIniciado || examenTerminado) return;

  // FASE 3: Confirmación de entrega
  if (pantallaConfirmacion) {
    manejarConfirmacion();
    return;
  }

  // FASE 4: Navegación del Examen
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
