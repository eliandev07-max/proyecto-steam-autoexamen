# Documentación Técnica


Este documento contiene la documentación técnica para el proyecto STEAM del curso **Laboratorio STEAM+** de la tecnicatura **Redes y Software** del Instituto Superior Brazo Oriental de **UTU** año 2026.

# Proyecto: \*AUTOEXAMEN

## 1. Integrantes

- Elian Gutierrez
- Nicolas Rodriguez
- Richard Rodriguez
- Agustin Silva

## 2. Descripción

- Se trata de un dispositivo basado en micro-bit el cual automatice la corrección de examenes de múltiple opción, brindando una capa intermedia entre el papel y los dispositivos inteligentes (de los cuales se puede copiar facilmente).
- Buscamos crear un dispositivo que se encuentre entre el mundo digital de los formularios por computadora y el mundo a papel, eliminando el error humano y el tiempo de corrección, agilizando procesos y en el camino ahorrando en costos e impacto ambiental en el uso del papel.

## 3. Materiales

- `ESP-32 DEV KIT 1.0`, `botones`, `batería`, `carcasa`, `pantalla lcd 1202`, `cables`, `protoboard`.

## 4. Diseño Mecánico

- [Realizar una descripción de cómo funciona la solución a nivel mecánico]
- [¿Cómo se arma la solución? Incluir instrucciones de ensamblaje]
  - [Sugerencia: inspirarse en la forma como https://www.instructables.com detalla el armado de un proyecto]
- [Incluir fotografías ilustrativas]

## 5. Diseño Electrónico

- [Realizar una descripción de como funciona la solución a nivel electrónico]
- [Incluir diagramas de Tinkercad mostrando los componentes electrónicos y como van conectados]

## 6. Diseño Software

- [Realizar una descripción de como funciona la solución a nivel de software]
- [Explicar los bloques de código más importantes de la solución programada]
- [Incluir código fuente de python (u otro lenguaje)]
  - [El código debe almacenarse en una carpeta dentro del repositorio]
  - [Sugerencia: almacenar el código en diferentes etapas para mostrar su evolución]

## 7. Referencias y recursos

[Listar referencias, fuentes, recursos, etc usados durante el desarrollo del proyecto]

## 8. Otros

[Incluir cualquier otra información que consideren relevante para el proyecto]

---

## Nota

Diagrama de la conexiónes importantes

**pines de 1602 I2C con esp32:**

<table border='1' style='text-align:center;'>
  <thead style='background-color:rgba(255,0,0,0.2)'>
  <td><u><b>1602 I2C</b></u></td>
  <td><u><b>esp32</b></u></td>
  </thead>
  <tbody>
  <tr>
  <td>GND</td>
  <td>GND</td>
  </tr>
  <tr><td>VCC</td><td>bin (5v)</td></tr>
  <tr>
  <td>SDA</td><td>d21</td></tr>
  <tr><td>SCL</td><td>d22</td></tr>

gnd - gnd
vcc - 5v
sda - d21
scl - d22

**pines de la botonera al esp32**

<table border='1' style='text-align:center;'>
  <thead style='background-color:rgba(255,0,0,0.2)'>
  <td><u><b>botonera</b></u></td>
  <td><u><b>esp32</b></u></td>
  </thead>
  <tbody>
  <tr>
  <td>SUBIR</td>
  <td>d25</td>
  </tr>
  <tr>
  <td>BAJAR</td>
  <td>d27</td>
  </tr>
  <tr>
  <td>CONFIRMAR</td>
  <td>d32</td>
  </tr>
  <tr>
  <td>IZQUIERDA</td>
  <td>d26</td>
  </tr>
  <tr>
  <td>DERECHA</td>
  <td>d13</td>
  </tr>
  </tbody>
</table>

---
