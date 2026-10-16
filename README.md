<h1 align="center">🐾 Sistema IoT de Alimentación para Mascotas</h1>
<h3 align="center">Dispensador automatizado controlado mediante Telegram</h3>

<p align="center">
  <img src="https://img.shields.io/badge/IoT-Sistemas%20Embebidos-0A7EA4?style=for-the-badge" alt="IoT">
  <img src="https://img.shields.io/badge/ESP32-Wi--Fi-E7352C?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32">
  <img src="https://img.shields.io/badge/Arduino-UNO-00979D?style=for-the-badge&logo=arduino&logoColor=white" alt="Arduino UNO">
  <img src="https://img.shields.io/badge/Telegram-Bot-26A5E4?style=for-the-badge&logo=telegram&logoColor=white" alt="Telegram Bot">
  <img src="https://img.shields.io/badge/Lenguaje-C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" alt="C++">
</p>

Proyecto integrador de hardware y software desarrollado para la cátedra de **Arquitectura de Computadoras** (2.° año de Ingeniería en Informática, UGD).

El proyecto consiste en el diseño, maquetado y programación de un dispensador automatizado de alimento y agua para mascotas. Opera de forma remota a través de Internet: el usuario envía comandos desde un **bot de Telegram** para accionar mecanismos físicos, recibe alertas cuando faltan insumos y puede verificar la dispensación mediante sensores de presión.

---

## 📑 Índice

- [Contexto y créditos](#-contexto-del-proyecto-y-créditos)
- [Arquitectura de hardware](#-arquitectura-del-sistema-hardware)
- [Software y comunicaciones](#-lógica-de-software-y-comunicaciones)
- [Comandos del bot](#-funcionalidades-del-bot)
- [Archivos del repositorio](#-estructura-del-repositorio)
- [Configuración y puesta en marcha](#-configuración-y-puesta-en-marcha)

---

## 👥 Contexto del proyecto y créditos

Este trabajo refleja el **esfuerzo colaborativo de cuatro estudiantes** para resolver una problemática de logística doméstica mediante conceptos de sistemas embebidos, puertos de entrada/salida y comunicación serie.

**Equipo de desarrolladores:**

- E. Centurión
- L. De León
- **Tomás Moroz** — curador de este repositorio
- E. Ojeda

---

## 🏗️ Arquitectura del sistema (hardware)

El sistema sigue una arquitectura distribuida tipo **maestro-esclavo**, separando la conectividad de red del control de los actuadores físicos.

### 📡 1. Módulo de comunicaciones — ESP32 (maestro)

- Gestiona la conexión Wi-Fi y la interacción con la API de Telegram.
- Envía instrucciones de alto nivel al módulo de actuación.
- Procesa la respuesta del hardware para notificar el resultado al usuario.

### ⚙️ 2. Módulo de actuación y E/S — Arduino UNO (esclavo)

- Recibe comandos del ESP32 por comunicación serie UART a **9600 baudios**.
- Lee las señales de los sensores, configurados mediante `INPUT_PULLUP`.
- Controla los actuadores electromecánicos y devuelve el estado de las operaciones.

### 🔩 Componentes físicos

| Componente | Función |
|---|---|
| **Motor paso a paso 28BYJ-48 + ULN2003** | Controla un disco dosificador impreso en 3D para servir raciones de alimento. |
| **Mini bomba de agua DC** | Transfiere agua desde el depósito al bebedero mediante un módulo relé. |
| **Micro-switches de presión KW11-3Z** | Detectan la entrega de alimento o agua y el vaciado de los depósitos para activar alertas de recarga. |

---

## 💻 Lógica de software y comunicaciones

### 📶 Portal cautivo de configuración

Cuando el ESP32 no detecta credenciales de red, levanta temporalmente una red Wi-Fi propia llamada `AlimentadorConfig`. Mediante un servidor web embebido en HTML, permite configurar el SSID, la contraseña, el token del bot y el *Chat ID* sin tener que reprogramar la placa. La configuración se conserva en la memoria flash mediante la biblioteca `Preferences`.

### 🔄 Comunicación asíncrona

El ESP32 y el Arduino intercambian cadenas de texto serializadas. Durante la dispensación, el Arduino bloquea nuevas operaciones mediante el estado `enProgreso` para evitar la superposición de comandos. Devuelve respuestas como `ALIMENTACION_COMPLETADA_CONFIRMADA` y alertas de falta de insumos.

### 🔐 Seguridad de acceso

El bot compara estrictamente el `sender.id` con el `chatID` almacenado, bloqueando comandos de usuarios no autorizados.

> **Importante:** no publiques tokens del bot, contraseñas Wi-Fi ni otros secretos en el repositorio. Configurá los datos de acceso en el dispositivo y evitá subir credenciales reales a los archivos fuente.

---

## 🤖 Funcionalidades del bot

| Comando | Funcionamiento |
|---|---|
| **`/comida`** | Comprueba si hay alimento en el depósito. Si hay, acciona el motor paso a paso y espera la confirmación del sensor del plato. Informa el éxito o alerta si el depósito está vacío. |
| **`/agua`** | Verifica la disponibilidad de agua, activa la bomba por un tiempo controlado y monitorea el switch del bebedero para confirmar la entrega y ayudar a prevenir desbordamientos. |

---

## 📂 Estructura del repositorio

- **`ARDUINO.ino`** — lógica de control del hardware, lectura de micro-switches, uso de `Stepper.h` y control de la bomba de agua.
- **`ESP.ino`** — conectividad, servidor web de configuración inicial, manejo de memoria flash, cliente de Telegram mediante `CTBot.h` y envío de tramas UART.
- **`Alimentador_y_Dispenser_de_Mascotas.pdf`** — informe técnico detallado, diagrama de Gantt y recursos utilizados durante la investigación.

---

## 🚀 Configuración y puesta en marcha

1. Abrí los archivos `.ino` en el **Arduino IDE**.
2. Configurá y seleccioná la placa correspondiente para cada sketch: ESP32 para `ESP.ino` y Arduino UNO para `ARDUINO.ino`.
3. Instalá o verificá las bibliotecas requeridas por el proyecto, incluidas `CTBot.h` y `Stepper.h`.
4. Conectá el hardware según el esquema de conexiones del informe técnico y cargá cada sketch en su placa correspondiente.
5. Encendé el sistema y, si el ESP32 no cuenta con credenciales Wi-Fi, conectate a la red `AlimentadorConfig` para realizar la configuración inicial.

> **Nota:** verificá las conexiones eléctricas y la alimentación de los motores y la bomba antes de encender el equipo. No se deben conectar cargas que superen las capacidades de las salidas de la placa.

---

<p align="center"><b>Arquitectura de Computadoras · IoT · ESP32 · Arduino · Telegram</b></p>
