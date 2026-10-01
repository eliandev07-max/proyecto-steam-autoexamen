# Informe de Avance 2: Septiembre 2026

## 2/9/2026

### Problemas encontrados

***1.0*** Problemas en la conección de redes.

Surgen problemas en la conección entre el esp32 y el dispositio que va a recepcionar el resultado

### Soluciones alternativas

***1.0*** Enlace directo desde el esp32 al dispositivo

Enlazamos directamente el esp32 mediante un punto hostpot al dispositivo que va a recibir las respuestas de los examenes, de ésta forma evitamos las restricciónes del wifi ceibal, el cual bloquea muchos puertos y protocoles en la red.

### Avances 

Nuevo programa monolito que gestiona (lcd 1602 + gestion del esp32 + de la recepción de las respuestas del examen) en [código completo v1.0 c++](../codigo%20de%20prueba/CodigoCompleto-Conexion.c++).

### Tareas completadas
  
  Implementamos una vista tipo página web para el control del profesor:

[Agregar nuevo alumno](../imagenes_proyecto/image%20(1).png)
  *** (Agregar nuevo alumno) ***
[Configuracion de la materia y examen](../imagenes_proyecto/image%20(2).png)
  *** (Configuración de la materia y examen) ***
[Control Examen](../imagenes_proyecto/image%20(3).png)
  *** (Establece las respuestas correctas y espera el resultado del examen) ***
[Agregar nuevo alumno](../imagenes_proyecto/image.png)
  *** (Ver las notas globales de cada alumno en el examen) ***

## 9/9/2026

No hubo clase

## 16/9/2026

Hoy realizamos trabajo de investigación, documentación y testing de las funcionalidades nuevas.

### Desafíos encontrados

- ****1.0**** Hoy probamos almacenar las respuestas y los alumnos en una base de datos en un maquina virtul rocky linux, el problema es con la conexión del servidor de base de datos y el esp32.
- ****2.0**** No se logró conectar el esp32 con la BD...

### Desafíos resueltos

- ****1.0**** Se resuelve con cambiar el adaptador de red de la maquina virtual de NAT a puente, de ésta forma se reconoce la ip de la VM desde el Sistema Operativo host.

### Tareas completadas

Dejamos andando el sistema de conexión entre el dispositivo del profesor y la esp.

### Futuras tareas

- Testear funcionalidades de la conección a l base de datos.
- Implementar la botonera física

## 23/9/2026

No hubo clase

## 30/9/2026

Hoy implementamos la nueva botonera con su case y los botones andando:

### Desafíos encontrados

* **1.0** Problemas con la implementación de la botonera, en el código (los cambles de los botones y los pines no tenían un orden), el botón de confirmar no funcionaba.  
* **2.0** Problemas con la selección de alumnos antes de empezar el examen.
### Desafíos resueltos

* **1.0** Acomodar los cables de la botonera para hacer match con los botónes.
* **2.0** Botón UP en falso contacto.

### Proximos pasos

Ya mandamos a imprimir el case oficial del proyecto

https://github.com/user-attachments/assets/38a6b14d-ed94-43bb-9fa4-cd2f9a2c67c9

### Imagenes relevantes

![Parte delantera de la botonera](../imagenes_proyecto/adelante_botonera.jpg)
**(Parte delantera de la botonera)**

![Parte trasera de la botonera](../imagenes_proyecto/atras_botonera.jpeg)
**(Parte trasera de la botonera)**

