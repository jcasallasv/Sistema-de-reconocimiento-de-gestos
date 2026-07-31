# Sistema Embebido Inalámbrico de Reconocimiento de Gestos

## Autores
- Julián Camilo Casallas (jcasallasv@unal.edu.co)
- Miguel Angel Cleves   (mcleves@unal.edu.co)
- Salomón Velasco Rueda (svelascor@unal.edu.co)

---

## Descripción
El Sistema Inalámbrico de Reconocimiento de Gestos consiste en el diseño e implementación de un sistema embebido inalámbrico capaz de reconocer gestos mediante la adquisición y procesamiento de señales provenientes de sensores de movimiento.
El sistema utiliza un microcontrolador basado en ESP32-WROOM-32D para capturar y procesar datos en tiempo real, los cuales son transmitidos de forma inalámbrica hacia una unidad de procesamiento central basada en Raspberry Pi Zero W.

En la Raspberry Pi se realiza el procesamiento adicional de los datos, así como la interpretación de patrones de movimiento para la identificación de gestos. El sistema puede generar respuestas como activación de actuadores, visualización de información o ejecución de acciones específicas según el gesto detectado.

---

## Objetivo General
Diseñar e implementar un sistema embebido inalámbrico que permita el reconocimiento de gestos en tiempo real mediante el uso de sensores de movimiento, procesamiento de datos y comunicación entre dispositivos.

---

## Objetivos Específicos
- Implementar la adquisición de señales de movimiento mediante sensores conectados al ESP32  
- Procesar y filtrar los datos obtenidos para la identificación de patrones de gestos  
- Diseñar una comunicación inalámbrica eficiente entre el ESP32 y la Raspberry Pi  
- Implementar un sistema de visualización o respuesta basado en los gestos reconocidos  
- Integrar técnicas básicas de clasificación (IA sencilla o reglas)  

---

## Arquitectura del Sistema


---

### Comunicación
- I2C (sensores y display)
- GPIO
- WiFi 

---

### Componentes

### Hardware

El sistema emplea los siguientes componentes físicos:

#### Microcontrolador
- ESP32-WROOM-32D

<img src="./Imagenes/ESP32.jpeg" width="15%">


#### Microprocesador
- Raspberry Pi Zero W

<img src="./Imagenes/RPIZEROW.jpeg" width="20%">


#### Sensores
- Sensor de movimiento MPU6050
- Sensor de gestos APDS-9960

<img src="./Imagenes/MPU6050.jpeg" width="19%">
<img src="./Imagenes/APDS9960.jpeg" width="16%">

#### Actuadores

- LEDs 
- Buzzer
- Pantalla OLED SSD1306
- Servomotor SG90

<img src="./Imagenes/Pantalla.jpeg" width="15%">
<img src="./Imagenes/Servo.jpeg" width="15%">

### Software

El sistema se desarrolla utilizando diferentes herramientas de software:

#### ESP32:
- Programación en Arduino IDE o PlatformIO
- Lectura de sensores
- Procesamiento básico de señales
- Envío de datos vía WiFi
#### Raspberry Pi:
- Sistema operativo Linux
- Python
- Framework Flask para recepción de datos
- Procesamiento y clasificación de gestos

### Cambios en el diseño del circuito 

Durante el desarrollo del proyecto se realizaron diversas modificaciones en el diseño del circuito, con el objetivo de mejorar la integración de los componentes, optimizar las conexiones y garantizar el correcto funcionamiento del sistema.

Inicialmente, se planteó un diseño preliminar enfocado en la conexión básica de los sensores y actuadores al microcontrolador. Sin embargo, a medida que se avanzó en la implementación, se identificaron limitaciones relacionadas con la distribución de pines, la alimentación de los dispositivos y la organización general del circuito.

Por esta razón, se llevaron a cabo ajustes en la conexión de periféricos, la asignación de pines del ESP32 y la integración de los módulos de comunicación y visualización.

A continuación, se presentan las diferentes iteraciones del diseño:

<p align="center">
  <img src="./Imagenes/Circuito.jpeg" width="38%">
  <img src="./Imagenes/Circuito2.jpeg" width="46%">
</p>

Finalmente, se obtuvo un diseño definitivo que integra todos los componentes del sistema de manera funcional y organizada, cumpliendo con los requerimientos del proyecto:

<p align="center">
  <img src="./Imagenes/Circuito3.jpeg" width="60%">
</p>

---

## Estructura del Proyecto
