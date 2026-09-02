Informe de Avance 1: Agosto 2026

## Aclaraciónes del desarrollo documentativo del proyecto

En markdown se pueden hacer tablas. Usar markdown siempre que sea posible, y solo HTML cuando no quede otra opción.


| Miembro    | Rol     | Descripción |
| ---------- | ------- | ----------- |
| Elian      | bla     | el rol es bla la vla |
| Agustin    | otro    | este otro rol tuki |



<table border='1' style='text-align:center;'>
<thead style='background-color:rgba(255,0,0,0.2)'>
<td><u><b>Miembro</b></u></td>
<td><u><b>Rol</b></u></td>
<td><u><b>Descripción</b></u></td>
</thead>
<tr>
<td>Elian Gutierrez</td>
<td>Documenter & investigador</td>
<td>Encargado de la documentación e investigación referente al proyecto</td>
</tr>
<tr>
<td>Agustin Silva</td>
<td>Investigador & Tester</td>
<td>Encargado de la investigación referente al proyecto y el testing de los diagramas y código</td>
</tr>
<tr>
<td>Nicolas Rodriguez</td>
<td>Orquestador & programador</td>
<td>Encargado de la orquesta de procesos y programación según los requerimientos funcionales</td>
</tr>
<tr>
<td>Richard Rodriguez</td>
<td>Electrónica & Soldadura</td>
<td>Encargado de la orquesta de procesos de electrónica aplicada y componentes</td>
</tr>
</table>

## 12/8/2026

En ésta clase realizamos un estudio de mercado conjunto con todos los integrantes del equipo para reconocer componentes, precios, calidad y compatibilidad en relación al objetivo del proyecto.

### Tareas completadas

- Buscamos y comparamos precios de componentes.
- Confirmamos el objetivo del proyecto.
- Repartimos tareas según habilidades.

### Problemas encontrados y soluciones/alternativas propuestas

- Planteamos la solución según las distintas plataformas a disposición `microbit`, `arduino uno` y `esp-32`
- Incompatibilidad de componentes

  Encontramos que usar microbit planteaba una arquitectura de nodos conectados a un gateway.
  El problema no es la arquitectura sino que la capacidad limitada de envío y recepción de
  datos por segundo que tiene la antena de radio de micro:bit, lo cual retrasaría y
  plantearía una solución más compleja fuera del alcance de proyecto.

![Ejemplo de arquitectura](../imagenes_proyecto/MicroBit.png)

#### Soluciónes alternativas

Barajamos las pataformas de arduino uno y esp 32, en primer lugar tomamos arduino uno por su adaptabilidad y ecosistema existente el cual nos proporciona un gran soporte en cuanto a librerias y tutoriales.

De cualquier forma deberíamos invertir en un modulo extra que nos sirva como antena entre cada nodo y el gateway, de ésta forma se sigue compejizando por incorporar modulos extras.

#### Solución final

![esp-32 mediante protocolo esp-now](../imagenes_proyecto/esp_to_pc2.png)

Utilizamos esp-32 con el protocolo esp-now para conectar los dispositivos nodos directamente a una coputadora que procese los datos de entrega del examen de cada dispositivo. De ésta forma planteamos una arquitectura más simple donde cada nodo se coneta directamente a una pc/laptop apuntando a su dirección MAC mediante el protocolo esp-now, así cuando los alumnos terminen el examen el resultado se envía directamente, sin intermediarios que retrasen el envío de datos y fecilite la escalabilidad cuando existan +50 nodos.

### Próximos pasos

- Integrar diagramas con la placa esp-32 física junto a una pantalla lcd 1602 I2C.

## 19/8/2026

En ésta clase nos ponemos manos a la obra, comenzamos los diagramas, el armado y primeros testeos

## Tareas completadas

- Primer testeo del lcd 1602 alimentada con el esp-32
- Primeros diagramas de conección del lcd 1602 con el esp-32
- Versión de [prueba del código en c++](codigo_de_prueba/ejemplo.c++)

## Problemas encontrados y soluciones/alternativas propuestas

### Compatibilidad

Desde el primer momento encontramos problemas en la compatibilidad entre las coputadoras con ide `Thonny` y `Arduino UNO` en relación al reconocimiento del esp-32 donde ejecutamos el código en micro python.

![testeando_esp32 con disferentes IDEs](../imagenes_proyecto/testeando_esp32.jpg)

### Solución de compatibilidad

![imagen de lcd andando conjunto al esp-32](../imagenes_proyecto/lcd_funcionando.jpg)

Tuvimos que instalar un driver de compatibilidad entre el esp-32 y las laptops llamado `CP210X UNIVERSAL WINDOWS`.

- Además de ésto implementamos el diagrama completo del esp32, el lcd 1602 I2C y la botonera:

![diagrama de esp32, botonera y lcd 1602 I2C](../imagenes_proyecto/diagrama_todo.jpg)

### Próximos pasos

- Manos a la obra con el modelado del circuito electrónico sobre protoboard.
- Ejecución del código en el esp32.
- Interactuar LCD 1206 I2C con el esp32 mediante código.

## 26/8/2026

- Hoy testeamos el codigo, resolvimos problemas de conecciónes, planteamos

### Problemas encontrados y soluciones/alternativas propuestas

#### Problema ####

- **1.0** El esp32 no entra en una protoboard completa, es necesario para tener disponible todos los pines.
- **2.0** El lcd 1602 no refleja los cambios del código del esp32.
  ![esp32 con el lcd ](../imagenes_proyecto/esp_testeandose.jpg)

  ### Soluciónes alternativas
- **1.0** pusimos el esp32 en la parte que necesitamos contectar más pines y del otro lado conectamos con cables hembra:
  ![esp32 que no funciona](../imagenes_proyecto/esp_no_entra.jpg)
  *esp32 conectado para funcionar*
- **2.0** Fallo de la conección del pin `SCL` del lcd con el pin `d22` del esp32, el problema estaba en un falso contacto de un cable flojo que hacía la conección. Lo solucionamos conecandolo directamente al esp32 cambiandole el cable en mal estado por uno en buen estado.
    ![funcionando](../imagenes_proyecto/funcionnando_2.jpg) 

### Próximos pasos

Sincronizar esp32 con pc laptop para envío de json y html.
