#include <Stepper.h>

const int pasosPorRevolucion = 2038;
const int pinMotor1 = 8;
const int pinMotor2 = 9;
const int pinMotor3 = 10;
const int pinMotor4 = 11;
Stepper miMotor(pasosPorRevolucion, pinMotor1, pinMotor3, pinMotor2, pinMotor4);

// --- Pines de sensores y bomba ---
const int pinAlmacenComida = 2;
const int pinPlatoComida = 3;
const int pinAlmacenAgua = 4;
const int pinPlatoAgua = 5;
const int bombaAgua = 6;

bool enProgreso = false;
const int pasosPorPorcion = 512;
const int tiempoEsperaPlato = 3000;

void setup() {
  Serial.begin(9600);

  pinMode(pinAlmacenComida, INPUT_PULLUP);
  pinMode(pinPlatoComida, INPUT_PULLUP);
  pinMode(pinAlmacenAgua, INPUT_PULLUP);
  pinMode(pinPlatoAgua, INPUT_PULLUP);
  pinMode(bombaAgua, OUTPUT);

  // Relay activo en LOW (apagado por defecto)
  digitalWrite(bombaAgua, LOW);

  miMotor.setSpeed(10);
  Serial.println("✅ Sistema listo - Switches conectados con INPUT_PULLUP a GND");
}

void loop() {
  if (Serial.available() > 0) {
    // Cambio aquí: readStringUntil('\n') para recibir los comandos inmediatamente
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();

    if (comando == "ALIMENTAR" && !enProgreso) {
      servirComida();
    } 
    else if (comando == "AGUA" && !enProgreso) {
      servirAgua();
    }
  }
  delay(50);
}

// ---------- SERVICIO DE AGUA ----------
void servirAgua() {
  enProgreso = true;
  Serial.println("💧 Iniciando servicio de agua...");

  bool hayAgua = switchPresionado(pinAlmacenAgua);
  bool platoConAgua = switchPresionado(pinPlatoAgua);

  if (!hayAgua) {
    Serial.println("🚫 Sin agua en el almacén");
  } else {
    Serial.println("📦 Depósito de agua disponible");

    if (platoConAgua) {
      Serial.println("💧 Agua cargada en el plato");
      Serial.println("✅ Agua servida y detectada en el plato");
    } else {
      Serial.println("⚙ Bomba accionada, agua aún no detectada en el plato");
      digitalWrite(bombaAgua, HIGH);   // activa bomba
      delay(1000);
      digitalWrite(bombaAgua, LOW);    // apaga bomba

      delay(tiempoEsperaPlato);
      platoConAgua = switchPresionado(pinPlatoAgua);

      if (platoConAgua) {
        Serial.println("✅ Agua servida y detectada en el plato");
      } else {
        Serial.println("⚠ Bomba accionada pero el plato sigue vacío");
      }
    }
  }

  enProgreso = false;
}

// ---------- SERVICIO DE COMIDA ----------
void servirComida() {
  enProgreso = true;
  Serial.println("🍽 Iniciando servicio de comida...");

  bool almacenComidaOK = switchPresionado(pinAlmacenComida);
  bool platoConComida = switchPresionado(pinPlatoComida);

  if (almacenComidaOK) {
    Serial.println("✅ Almacén comida OK");

    if (platoConComida) {
      Serial.println("🍽 Plato ya tiene comida");
      Serial.println("ALIMENTACION_COMPLETADA_CONFIRMADA - Comida ya presente");
    } else {
      Serial.println("🔄 Activando motor...");
      miMotor.step(pasosPorPorcion);
      delay(5000);
      miMotor.step(-pasosPorPorcion);

      delay(tiempoEsperaPlato);
      platoConComida = switchPresionado(pinPlatoComida);

      if (platoConComida) {
        Serial.println("ALIMENTACION_COMPLETADA_CONFIRMADA - Comida llegó al plato");
      } else {
        Serial.println("⚠ Motor activado, pero no se detectó comida en el plato");
      } 
    }
  } else {
    Serial.println("❌ No hay comida en el depósito");
  }

  enProgreso = false;
}

// ---------- LECTOR DE SWITCH ----------
bool switchPresionado(int pin) {
  return digitalRead(pin) == LOW;  // LOW = presionado (a GND)
}