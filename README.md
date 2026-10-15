# 🐾 Sistema de Alimentación IoT para Mascotas vía Telegram

Proyecto Integrador de Hardware y Software desarrollado para la cátedra de **Arquitectura de Computadoras** (2.° año de Ingeniería en Informática, UGD).

Este proyecto consiste en el diseño, maquetado y programación de un dispensador automatizado de alimento y agua para mascotas. El sistema opera de forma remota a través de Internet, permitiendo al usuario enviar comandos desde un Bot de Telegram para accionar los mecanismos físicos, recibir alertas de falta de insumos y monitorear el éxito de la dispensación gracias a una red de sensores de presión.

---

## 👥 Contexto del Proyecto y Créditos

Este trabajo refleja el **esfuerzo colaborativo de 4 estudiantes** para resolver una problemática real de logística doméstica mediante la aplicación de conceptos de arquitectura de sistemas embebidos, diseño de puertos de E/S y comunicación serie.

**Equipo de Desarrolladores:**
* E. Centurión
* L. De León
* **Tomás Moroz** *(Curador de este repositorio)*
* E. Ojeda

---

## 🏗️ Arquitectura del Sistema (Hardware)

El sistema emplea una arquitectura distribuida tipo Maestro-Esclavo, separando las cargas de red de las cargas de actuación física:

1. **Módulo de Comunicaciones (ESP32 - Maestro):** 
   * Encargado de la conectividad Wi-Fi y la interacción directa con la API de Telegram. 
   * Envía instrucciones de alto nivel y procesa el *feedback* del hardware para notificar al usuario.
2. **Módulo de Actuación y E/S (Arduino UNO - Esclavo):** 
   * Recibe comandos del ESP32 vía comunicación Serial (UART a 9600 baudios). 
   * Procesa las señales de entrada de los sensores (configurados en `INPUT_PULLUP`) y controla los actuadores electromecánicos.

**Componentes Físicos:**
* **Motor Paso a Paso (28BYJ-48 + ULN2003):** Controla un disco dosificador impreso en 3D para dispensar raciones exactas de alimento.
* **Mini Bomba de Agua DC:** Accionada mediante un módulo Relay para transferir líquido del depósito al bebedero.
* **Micro-Switches de Presión (KW11-3Z):** Estratégicamente ubicados en el plato y los depósitos para detectar tanto el impacto del alimento/agua (confirmación de entrega) como el vaciado de los silos (alerta de recarga).

---

## 💻 Lógica de Software y Comunicaciones

* **Portal Cautivo de Configuración (Access Point):** El ESP32 levanta temporalmente una red Wi-Fi propia (`AlimentadorConfig`) si no detecta credenciales. A través de un servidor web embebido (HTML), permite al usuario configurar SSID, Password, Token del Bot y Chat ID sin necesidad de reprogramar la placa. Estos datos persisten en la memoria flash utilizando la librería `Preferences`.
* **Comunicación Asíncrona:** El ESP32 y el Arduino se comunican mediante un protocolo de *strings* serializados. El Arduino bloquea el estado (`enProgreso`) durante la dispensación para evitar la superposición de comandos, enviando confirmaciones como `ALIMENTACION_COMPLETADA_CONFIRMADA` o alertas como `🚫 Sin agua en el almacén`.
* **Seguridad de Acceso:** El Bot de Telegram verifica estrictamente el `sender.id` contra el `chatID` almacenado en memoria, bloqueando comandos de usuarios no autorizados.

---

## ⚙️ Funcionalidades del Bot

* `/comida` — El sistema verifica si hay alimento en la botella. Si hay, acciona el motor paso a paso y espera la confirmación del sensor del plato. Retorna mensajes de éxito o alertas de depósito vacío.
* `/agua` — Verifica el nivel de agua. Activa la bomba DC por un tiempo determinado hasta que el switch del bebedero se accione, previniendo desbordamientos y confirmando la entrega en el chat.

---

## 📂 Estructura del Repositorio

* `ARDUINO.ino` — Lógica de control de hardware, lectura de micro-switches, manejo de la librería `Stepper.h` y control de la bomba de agua.
* `ESP.ino` — Lógica de red, servidor web para configuración inicial (AP), manejo de la memoria EEPROM/Flash, cliente de Telegram (`CTBot.h`) y envío de tramas UART.
* `Alimentador_y_Dispenser_de_Mascotas.pdf` — Informe técnico detallado, diagrama de Gantt y recursos utilizados durante la investigación.
