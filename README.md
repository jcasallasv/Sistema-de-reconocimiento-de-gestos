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

#### Microprocesador
- Raspberry Pi Zero W

#### Sensores
- Sensor de movimiento MPU6050
- LEDs 
- Buzzer
- Pantalla OLED

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

### Diseño del circuito 

---

## Estructura del Proyecto
