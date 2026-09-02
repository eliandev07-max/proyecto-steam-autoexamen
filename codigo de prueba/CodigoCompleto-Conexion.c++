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

// --- Credenciales del Profesor ---
const String USUARIO_CORRECTO = "ruso";
const String CLAVE_CORRECTA = "ruso2026";

// Estado de sesión activa
bool usuarioAutenticado = false;
String tokenSesion = "";

// Mapeo de pines
#define BTN_RIGHT  13
#define BTN_LEFT   26
#define BTN_UP     25
#define BTN_DOWN   27
#define BTN_SELECT 32

const int MAX_PREGUNTAS = 50;
const int MAX_ALUMNOS = 30;

int totalPreguntas = 10;
int respuestaCorrecta[MAX_PREGUNTAS];
int respuestaAlumno[MAX_PREGUNTAS]; 

String nombreMateria = "Matematica";

// Lista de alumnos dinámica
int totalListaAlumnos = 3;
String listaAlumnos[MAX_ALUMNOS] = {
  "Agustin",
  "Maria",
  "Juan"
};

int indiceAlumnoSeleccionado = 0;
String alumnoActual = "";
bool alumnoConfirmado = false; 

int preguntaActual = 0;
bool pantallaConfirmacion = false;
bool examenTerminado = false;
bool examenIniciado = false; 

unsigned long ultimoBoton = 0;
const int debounceDelay = 200;

char letras[4] = {'A', 'B', 'C', 'D'};

// Estructura para registrar notas asociadas a la materia
struct RegistroNota {
  String nombre;
  String materia;
  int puntaje;
  int totalPreguntasExamen;
  bool realizado;
  int respuestasEnviadas[MAX_PREGUNTAS];
};

RegistroNota historialNotas[MAX_ALUMNOS];

// ================= DECLARACIÓN DE PROTOTIPOS =================
void mostrarSeleccionAlumno();
void mostrarEsperandoProfe();
void mostrarPregunta();
void mostrarConfirmacion();
void manejarConfirmacion();
void moverOpcion(int dir);
void moverPregunta(int dir);
void finalizarExamen();
bool verificarAutenticacion();

// ================= SEGURIDAD Y AUTENTICACIÓN =================

bool verificarAutenticacion() {
  if (!usuarioAutenticado) return false;
  
  if (server.hasHeader("Cookie")) {
    String cookie = server.header("Cookie");
    if (cookie.indexOf("SESSIONID=" + tokenSesion) != -1 && tokenSesion.length() > 0) {
      return true;
    }
  }
  return false;
}

// ================= VISTAS Y FUNCIONES LCD =================

void mostrarSeleccionAlumno() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Elegir alumno:");
  lcd.setCursor(0, 1);
  if (totalListaAlumnos > 0) {
    lcd.print(">");
    lcd.print(listaAlumnos[indiceAlumnoSeleccionado]);
  } else {
    lcd.print("Sin alumnos");
  }
}

void mostrarEsperandoProfe() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hola ");
  lcd.print(alumnoActual.substring(0, 11));
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
  lcd.print(alumnoActual.substring(0, 8));

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
    historialNotas[indiceAlumnoSeleccionado].respuestasEnviadas[i] = respuestaAlumno[i];
  }

  historialNotas[indiceAlumnoSeleccionado].materia = nombreMateria;
  historialNotas[indiceAlumnoSeleccionado].puntaje = puntaje;
  historialNotas[indiceAlumnoSeleccionado].totalPreguntasExamen = totalPreguntas;
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

// ================= PAGINAS HTML =================

String paginaLogin(bool error = false) {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Autoexamen - Iniciar Sesion</title>
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #0f172a; color: #f8fafc; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
    .login-card { background: #1e293b; padding: 30px; border-radius: 10px; border: 1px solid #334155; width: 100%; max-width: 360px; box-shadow: 0 4px 15px rgba(0,0,0,0.3); }
    h2 { text-align: center; margin-bottom: 20px; color: #38bdf8; font-size: 22px; }
    .form-group { margin-bottom: 15px; }
    label { display: block; margin-bottom: 5px; font-size: 14px; color: #94a3b8; }
    input[type=text], input[type=password] { width: 100%; padding: 10px; border: 1px solid #334155; border-radius: 6px; background: #0f172a; color: white; box-sizing: border-box; font-size: 14px; }
    button { width: 100%; padding: 10px; background: #0284c7; color: white; border: none; border-radius: 6px; font-size: 15px; font-weight: bold; cursor: pointer; margin-top: 10px; }
    button:hover { background: #0369a1; }
    .error { color: #ef4444; text-align: center; font-size: 13px; margin-bottom: 15px; background: rgba(239, 68, 68, 0.1); padding: 8px; border-radius: 4px; }
  </style>
</head>
<body>
  <div class="login-card">
    <h2>Acceso Profesor</h2>
)rawliteral";

  if (error) {
    html += "<div class='error'>Usuario o contraseña incorrectos.</div>";
  }

  html += R"rawliteral(
    <form action="/login" method="POST">
      <div class="form-group">
        <label>Usuario</label>
        <input type="text" name="usuario" required autocomplete="off">
      </div>
      <div class="form-group">
        <label>Contraseña</label>
        <input type="password" name="password" required>
      </div>
      <button type="submit">Ingresar</button>
    </form>
  </div>
</body>
</html>
)rawliteral";
  return html;
}

String paginaPrincipal() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Autoexamen - Panel del Profesor</title>
  <style>
    :root {
      --bg-color: #f1f5f9;
      --card-bg: #ffffff;
      --text-color: #0f172a;
      --border-color: #e2e8f0;
      --nav-bg: #0f172a;
      --subtext: #64748b;
    }

    body.dark {
      --bg-color: #0f172a;
      --card-bg: #1e293b;
      --text-color: #f8fafc;
      --border-color: #334155;
      --nav-bg: #020617;
      --subtext: #94a3b8;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: var(--bg-color); color: var(--text-color); transition: background 0.3s, color 0.3s; }
    
    nav { background-color: var(--nav-bg); color: white; display: flex; justify-content: space-between; align-items: center; padding: 0 20px; height: 60px; box-shadow: 0 2px 8px rgba(0,0,0,0.15); }
    .nav-brand { font-size: 18px; font-weight: bold; color: #38bdf8; display: flex; align-items: center; gap: 8px; }
    .nav-actions { display: flex; gap: 10px; align-items: center; }
    .theme-toggle { background: #334155; border: none; color: white; padding: 6px 12px; border-radius: 20px; cursor: pointer; font-size: 13px; font-weight: bold; }
    .logout-btn { background: #ef4444; color: white; text-decoration: none; padding: 6px 12px; border-radius: 20px; font-size: 13px; font-weight: bold; }

    .container { max-width: 850px; margin: 20px auto; padding: 0 15px; }
    
    .tabs { display: flex; border-bottom: 2px solid var(--border-color); margin-bottom: 20px; gap: 5px; }
    .tab-btn { background: none; border: none; padding: 10px 18px; font-size: 15px; font-weight: 600; color: var(--subtext); cursor: pointer; border-bottom: 3px solid transparent; transition: all 0.2s; }
    .tab-btn.active { color: #0284c7; border-bottom-color: #0284c7; }

    .tab-content { display: none; }
    .tab-content.active { display: block; }

    .card { background: var(--card-bg); padding: 22px; border-radius: 10px; border: 1px solid var(--border-color); box-shadow: 0 2px 5px rgba(0,0,0,0.05); margin-bottom: 15px; }
    h2 { font-size: 18px; margin-bottom: 15px; border-bottom: 2px solid var(--border-color); padding-bottom: 8px; color: var(--text-color); }
    
    .form-group { margin-bottom: 15px; }
    .form-group label { display: block; font-weight: 600; margin-bottom: 6px; font-size: 14px; }
    .form-group input[type=text], .form-group input[type=number], .form-group textarea { width: 100%; padding: 10px; border: 1px solid var(--border-color); border-radius: 6px; background: var(--bg-color); color: var(--text-color); font-size: 14px; }

    .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(120px, 1fr)); gap: 10px; margin-top: 15px; }
    .item { background: var(--bg-color); border: 1px solid var(--border-color); padding: 8px; border-radius: 6px; text-align: center; }
    .item span { font-weight: bold; display: block; margin-bottom: 5px; font-size: 13px; color: var(--subtext); }
    .options { display: flex; justify-content: space-around; }

    button.btn-action { padding: 10px 16px; cursor: pointer; font-size: 14px; border-radius: 6px; font-weight: bold; border: none; transition: opacity 0.2s; width: 100%; margin-top: 10px; }
    button.btn-action:hover { opacity: 0.9; }
    .guardar { background: #0284c7; color: white; }
    .iniciar { background: #16a34a; color: white; }
    .eliminar { background: #ef4444; color: white; padding: 4px 8px; font-size: 12px; border-radius: 4px; border: none; cursor: pointer; }
    
    .status-box { font-size: 16px; text-align: center; font-weight: bold; padding: 15px; border-radius: 8px; margin-bottom: 15px; }
    .esperando { background: #fef3c7; color: #b45309; }
    .listo { background: #e0f2fe; color: #0369a1; }
    .finalizado { background: #dcfce7; color: #15803d; }
    
    .respuesta-tag { display: inline-block; padding: 2px 6px; border-radius: 4px; font-size: 12px; margin: 2px; font-weight: bold; }
    .correcta { background: #22c55e; color: white; }
    .incorrecta { background: #ef4444; color: white; }

    table { width: 100%; border-collapse: collapse; margin-top: 10px; }
    th, td { border: 1px solid var(--border-color); padding: 10px; text-align: center; font-size: 14px; }
    th { background-color: var(--bg-color); color: var(--subtext); }
    
    .alumno-table-list td { text-align: left; }
    .materia-card { border: 1px solid var(--border-color); border-radius: 8px; margin-bottom: 12px; overflow: hidden; }
    .materia-header { background: var(--bg-color); padding: 12px 16px; font-weight: bold; cursor: pointer; display: flex; justify-content: space-between; align-items: center; }
    .materia-body { padding: 15px; display: none; background: var(--card-bg); }
    .materia-body.open { display: block; }
  </style>
</head>
<body>

  <nav>
    <div class="nav-brand">AutoExamen <span id="nombre-materia" style="color: #38bdf8; font-size: 14px; font-weight: normal;"></span></div>
    <div class="nav-actions">
      <button class="theme-toggle" onclick="toggleTheme()">Modo Oscuro</button>
      <a href="/logout" class="logout-btn">Cerrar Sesión</a>
    </div>
  </nav>

  <div class="container">
    
    <div class="tabs">
      <button class="tab-btn active" onclick="openTab('tab-control')">Control Examen</button>
      <button class="tab-btn" onclick="openTab('tab-clave')">Materia y Clave</button>
      <button class="tab-btn" onclick="openTab('tab-alumnos')">Alumnos</button>
      <button class="tab-btn" onclick="openTab('tab-materias')">Materias / Notas</button>
    </div>

    <!-- PESTAÑA 1: CONTROL EN VIVO -->
    <div id="tab-control" class="tab-content active">
      <div class="card">
        <h2>Estado del Alumno en LCD</h2>
        <div id="estado-contenedor" class="status-box esperando">Cargando estado...</div>
        <form action="/iniciar" method="POST">
          <button type="submit" id="btn-iniciar" class="btn-action iniciar" disabled>Iniciar Examen</button>
        </form>
      </div>

      <div class="card">
        <h2>Respuestas del Examen Activo</h2>
        <div id="desglose">Sin datos de examen activo.</div>
      </div>
    </div>

    <!-- PESTAÑA 2: CONFIGURACIÓN Y CLAVE -->
    <div id="tab-clave" class="tab-content">
      <div class="card">
        <h2>Configuración de la Materia</h2>
        <form action="/guardar_config" method="POST">
          <div class="form-group">
            <label for="materia">Nombre de la Materia:</label>
            <input type="text" id="materia" name="materia" value=")rawliteral";
  
  html += nombreMateria;

  html += R"rawliteral(" required>
          </div>
          <div class="form-group">
            <label for="cant_preguntas">Cantidad de Preguntas (1 a 50):</label>
            <input type="number" id="cant_preguntas" name="cant_preguntas" value=")rawliteral";
  
  html += String(totalPreguntas);

  html += R"rawliteral(" min="1" max="50" required>
          </div>
          <button type="submit" class="btn-action guardar">Guardar Materia y Preguntas</button>
        </form>
      </div>

      <div class="card">
        <h2>Clave de Respuestas Correctas</h2>
        <form action="/guardar_clave" method="POST">
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
          <button type="submit" class="btn-action guardar">Guardar Solucionario</button>
        </form>
      </div>
    </div>

    <!-- PESTAÑA 3: GESTIÓN DE ALUMNOS -->
    <div id="tab-alumnos" class="tab-content">
      <div class="card">
        <h2>Importar Lista Completa de Alumnos</h2>
        <form action="/importar_alumnos" method="POST">
          <div class="form-group">
            <label for="lista_importar">Pegar nombres (separados por coma o salto de línea):</label>
            <textarea id="lista_importar" name="lista_importar" rows="4" placeholder="Agustin&#10;Maria&#10;Juan, Lucas, Sofia"></textarea>
          </div>
          <button type="submit" class="btn-action guardar">Importar y Cargar en ESP32</button>
        </form>
      </div>

      <div class="card">
        <h2>Agregar Alumno Individual</h2>
        <form action="/agregar_alumno" method="POST" style="display:flex; gap:10px;">
          <input type="text" name="nuevo_nombre" placeholder="Nombre de alumno" maxlength="12" required style="padding:8px; border-radius:5px; border:1px solid var(--border-color); width:100%;">
          <button type="submit" class="btn-action guardar" style="margin-top:0; width:150px;">Agregar</button>
        </form>
      </div>

      <div class="card">
        <h2>Lista de Alumnos Cargados</h2>
        <div id="tabla-alumnos-gestion">Cargando...</div>
      </div>
    </div>

    <!-- PESTAÑA 4: APARTADO POR MATERIAS Y RESPUESTAS -->
    <div id="tab-materias" class="tab-content">
      <div class="card">
        <h2>Mapeo por Materias</h2>
        <p style="font-size:13px; color:var(--subtext); margin-bottom:15px;">Haz clic en una materia para expandir y ver los alumnos evaluados y sus respuestas.</p>
        <div id="contenedor-materias">Cargando materias...</div>
      </div>
    </div>

  </div>

  <script>
    function openTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(tab => tab.classList.remove('active'));
      document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
      
      document.getElementById(tabId).classList.add('active');
      event.currentTarget.classList.add('active');
    }

    function toggleTheme() {
      document.body.classList.toggle('dark');
      localStorage.setItem('theme', document.body.classList.contains('dark') ? 'dark' : 'light');
    }

    if (localStorage.getItem('theme') === 'dark') {
      document.body.classList.add('dark');
    }

    function toggleMateria(id) {
      const body = document.getElementById(id);
      body.classList.toggle('open');
    }

    function consultarEstado() {
      fetch('/api_estado')
        .then(response => {
          if (response.status === 401) {
            window.location.href = "/login";
            return;
          }
          return response.json();
        })
        .then(data => {
          if (!data) return;
          document.getElementById('nombre-materia').innerText = "| Materia Actual: " + data.materia;
          
          const contenedor = document.getElementById('estado-contenedor');
          const btnIniciar = document.getElementById('btn-iniciar');
          const desglose = document.getElementById('desglose');
          const gestionAlumnos = document.getElementById('tabla-alumnos-gestion');
          const contenedorMaterias = document.getElementById('contenedor-materias');

          if (!data.confirmado) {
            contenedor.className = "status-box esperando";
            contenedor.innerHTML = "Esperando que un alumno se seleccione en el LCD...";
            btnIniciar.disabled = true;
            btnIniciar.innerText = "Esperando alumno...";
            desglose.innerHTML = "Ningún alumno seleccionado.";
          } else if (!data.iniciado) {
            contenedor.className = "status-box listo";
            contenedor.innerHTML = "Alumno listo: <b>" + data.alumno + "</b>";
            btnIniciar.disabled = false;
            btnIniciar.innerText = "Habilitar Examen a " + data.alumno;
            desglose.innerHTML = "Esperando inicio del examen.";
          } else if (!data.terminado) {
            contenedor.className = "status-box esperando";
            contenedor.innerHTML = "En progreso: <b>" + data.alumno + "</b> respondiendo...";
            btnIniciar.disabled = true;
            btnIniciar.innerText = "Examen en Curso";
            desglose.innerHTML = "El alumno está respondiendo en el hardware...";
          } else {
            contenedor.className = "status-box finalizado";
            contenedor.innerHTML = "EXAMEN FINALIZADO<br>Alumno: <b>" + data.alumno + "</b><br>Puntaje: " + data.puntaje + " / " + data.total;
            btnIniciar.disabled = false;
            btnIniciar.innerText = "Siguiente Alumno";

            let htmlDesglose = "<div style='display:grid; grid-template-columns: repeat(auto-fill, minmax(80px, 1fr)); gap:5px;'>";
            data.respuestas.forEach((resp, idx) => {
              let clase = (resp.alumno === resp.correcta) ? "correcta" : "incorrecta";
              htmlDesglose += `<div style='border:1px solid var(--border-color); padding:4px; text-align:center; border-radius:4px;'>
                <b>P${idx + 1}:</b> <span class='respuesta-tag ${clase}'>${resp.alumno}</span>
              </div>`;
            });
            htmlDesglose += "</div>";
            desglose.innerHTML = htmlDesglose;
          }

          let htmlGestion = "<table class='alumno-table-list'><tr><th>Alumno</th><th style='width:80px;'>Acción</th></tr>";
          data.historial.forEach((h, idx) => {
            htmlGestion += `<tr>
              <td>${h.nombre}</td>
              <td style='text-align:center;'>
                <form action='/borrar_alumno' method='POST' style='display:inline;'>
                  <input type='hidden' name='index' value='${idx}'>
                  <button type='submit' class='eliminar'>Borrar</button>
                </form>
              </td>
            </tr>`;
          });
          htmlGestion += "</table>";
          gestionAlumnos.innerHTML = htmlGestion;

          let materiasMap = {};
          data.historial.forEach(h => {
            let mat = h.materia || "Sin Materia";
            if (!materiasMap[mat]) materiasMap[mat] = [];
            materiasMap[mat].push(h);
          });

          let htmlMaterias = "";
          let idCount = 0;
          for (let mat in materiasMap) {
            idCount++;
            let panelId = "mat_panel_" + idCount;
            htmlMaterias += `<div class="materia-card">
              <div class="materia-header" onclick="toggleMateria('${panelId}')">
                <span>${mat}</span>
                <span style="font-size:12px; font-weight:normal;">Ver Alumnos (${materiasMap[mat].length})</span>
              </div>
              <div id="${panelId}" class="materia-body">
                <table>
                  <tr><th>Alumno</th><th>Estado</th><th>Nota</th></tr>`;
            
            materiasMap[mat].forEach(al => {
              htmlMaterias += `<tr>
                <td>${al.nombre}</td>
                <td>${al.realizado ? 'Completado' : 'Pendiente'}</td>
                <td><b>${al.realizado ? al.puntaje + ' / ' + al.total : '-'}</b></td>
              </tr>`;
            });

            htmlMaterias += `</table></div></div>`;
          }
          contenedorMaterias.innerHTML = htmlMaterias;
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

// ================= HANDLERS DEL SERVIDOR =================

void handleRoot() {
  if (!verificarAutenticacion()) {
    server.sendHeader("Location", "/login");
    server.send(302, "text/plain", "");
    return;
  }
  server.send(200, "text/html", paginaPrincipal());
}

void handleLoginGet() {
  server.send(200, "text/html", paginaLogin(false));
}

void handleLoginPost() {
  if (server.hasArg("usuario") && server.hasArg("password")) {
    if (server.arg("usuario") == USUARIO_CORRECTO && server.arg("password") == CLAVE_CORRECTA) {
      usuarioAutenticado = true;
      tokenSesion = String(random(100000, 999999)); // Genera token único en servidor
      
      server.sendHeader("Set-Cookie", "SESSIONID=" + tokenSesion + "; Path=/; HttpOnly");
      server.sendHeader("Location", "/");
      server.send(302, "text/plain", "");
      return;
    }
  }
  server.send(200, "text/html", paginaLogin(true));
}

void handleLogout() {
  usuarioAutenticado = false;
  tokenSesion = "";
  server.sendHeader("Set-Cookie", "SESSIONID=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
  server.sendHeader("Location", "/login");
  server.send(302, "text/plain", "");
}

void handleGuardarConfig() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
  if (server.hasArg("materia")) {
    nombreMateria = server.arg("materia");
  }
  if (server.hasArg("cant_preguntas")) {
    int nuevaCant = server.arg("cant_preguntas").toInt();
    if (nuevaCant >= 1 && nuevaCant <= MAX_PREGUNTAS) {
      totalPreguntas = nuevaCant;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleGuardarClave() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
  for (int i = 0; i < totalPreguntas; i++) {
    String campo = "resp" + String(i);
    if (server.hasArg(campo)) {
      respuestaCorrecta[i] = server.arg(campo).toInt();
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleAgregarAlumno() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
  if (server.hasArg("nuevo_nombre") && totalListaAlumnos < MAX_ALUMNOS) {
    String nuevo = server.arg("nuevo_nombre");
    nuevo.trim();
    if (nuevo.length() > 0) {
      listaAlumnos[totalListaAlumnos] = nuevo;
      historialNotas[totalListaAlumnos].nombre = nuevo;
      historialNotas[totalListaAlumnos].materia = nombreMateria;
      historialNotas[totalListaAlumnos].puntaje = 0;
      historialNotas[totalListaAlumnos].totalPreguntasExamen = totalPreguntas;
      historialNotas[totalListaAlumnos].realizado = false;
      totalListaAlumnos++;
      
      if (!alumnoConfirmado) {
        mostrarSeleccionAlumno();
      }
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleImportarAlumnos() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
  if (server.hasArg("lista_importar")) {
    String raw = server.arg("lista_importar");
    totalListaAlumnos = 0;
    
    int len = raw.length();
    String temp = "";
    
    for (int i = 0; i < len; i++) {
      char c = raw.charAt(i);
      if (c == ',' || c == '\n' || c == '\r') {
        temp.trim();
        if (temp.length() > 0 && totalListaAlumnos < MAX_ALUMNOS) {
          listaAlumnos[totalListaAlumnos] = temp;
          historialNotas[totalListaAlumnos].nombre = temp;
          historialNotas[totalListaAlumnos].materia = nombreMateria;
          historialNotas[totalListaAlumnos].puntaje = 0;
          historialNotas[totalListaAlumnos].totalPreguntasExamen = totalPreguntas;
          historialNotas[totalListaAlumnos].realizado = false;
          totalListaAlumnos++;
        }
        temp = "";
      } else {
        temp += c;
      }
    }
    
    temp.trim();
    if (temp.length() > 0 && totalListaAlumnos < MAX_ALUMNOS) {
      listaAlumnos[totalListaAlumnos] = temp;
      historialNotas[totalListaAlumnos].nombre = temp;
      historialNotas[totalListaAlumnos].materia = nombreMateria;
      historialNotas[totalListaAlumnos].puntaje = 0;
      historialNotas[totalListaAlumnos].totalPreguntasExamen = totalPreguntas;
      historialNotas[totalListaAlumnos].realizado = false;
      totalListaAlumnos++;
    }

    indiceAlumnoSeleccionado = 0;
    alumnoConfirmado = false;
    mostrarSeleccionAlumno();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBorrarAlumno() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
  if (server.hasArg("index")) {
    int idx = server.arg("index").toInt();
    if (idx >= 0 && idx < totalListaAlumnos) {
      for (int i = idx; i < totalListaAlumnos - 1; i++) {
        listaAlumnos[i] = listaAlumnos[i + 1];
        historialNotas[i] = historialNotas[i + 1];
      }
      totalListaAlumnos--;
      
      if (indiceAlumnoSeleccionado >= totalListaAlumnos) {
        indiceAlumnoSeleccionado = max(0, totalListaAlumnos - 1);
      }
      
      if (!alumnoConfirmado) {
        mostrarSeleccionAlumno();
      }
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleIniciar() {
  if (!verificarAutenticacion()) { server.send(401, "text/plain", "No autorizado"); return; }
  
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
  if (!verificarAutenticacion()) {
    server.send(401, "application/json", "{\"error\":\"unauthorized\"}");
    return;
  }

  String json = "{";
  json += "\"confirmado\":" + String(alumnoConfirmado ? "true" : "false") + ",";
  json += "\"iniciado\":" + String(examenIniciado ? "true" : "false") + ",";
  json += "\"terminado\":" + String(examenTerminado ? "true" : "false") + ",";
  json += "\"alumno\":\"" + alumnoActual + "\",";
  json += "\"materia\":\"" + nombreMateria + "\",";
  
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

  String histArr = "[";
  for (int i = 0; i < totalListaAlumnos; i++) {
    histArr += "{\"nombre\":\"" + historialNotas[i].nombre + "\",";
    histArr += "\"materia\":\"" + historialNotas[i].materia + "\",";
    histArr += "\"realizado\":" + String(historialNotas[i].realizado ? "true" : "false") + ",";
    histArr += "\"puntaje\":" + String(historialNotas[i].puntaje) + ",";
    histArr += "\"total\":" + String(historialNotas[i].totalPreguntasExamen) + "}";
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

  for (int i = 0; i < totalListaAlumnos; i++) {
    historialNotas[i].nombre = listaAlumnos[i];
    historialNotas[i].materia = nombreMateria;
    historialNotas[i].puntaje = 0;
    historialNotas[i].totalPreguntasExamen = totalPreguntas;
    historialNotas[i].realizado = false;
  }

  for (int i = 0; i < MAX_PREGUNTAS; i++) {
    respuestaAlumno[i] = -1;   
    respuestaCorrecta[i] = 0;  
  }

  WiFi.softAP(ssid, password);
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Indicar al servidor que lea las cabeceras HTTP necesarias para validar las Cookies
  const char * headerkeys[] = {"Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
  server.collectHeaders(headerkeys, headerkeyssize);

  server.on("/", handleRoot);
  server.on("/login", HTTP_GET, handleLoginGet);
  server.on("/login", HTTP_POST, handleLoginPost);
  server.on("/logout", handleLogout);
  
  server.on("/guardar_config", HTTP_POST, handleGuardarConfig);
  server.on("/guardar_clave", HTTP_POST, handleGuardarClave);
  server.on("/agregar_alumno", HTTP_POST, handleAgregarAlumno);
  server.on("/importar_alumnos", HTTP_POST, handleImportarAlumnos);
  server.on("/borrar_alumno", HTTP_POST, handleBorrarAlumno);
  server.on("/iniciar", HTTP_POST, handleIniciar);
  server.on("/api_estado", HTTP_GET, handleApiEstado);
  
  server.onNotFound([]() {
    server.sendHeader("Location", "http://192.168.4.1/login");
    server.send(302, "text/plain", "");
  });

  server.begin();
  mostrarSeleccionAlumno();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient(); 

  if (millis() - ultimoBoton < debounceDelay) return;

  if (!alumnoConfirmado) {
    if (totalListaAlumnos == 0) return;

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

  if (!examenIniciado || examenTerminado) return;

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
