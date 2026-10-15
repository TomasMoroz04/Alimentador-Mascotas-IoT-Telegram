#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <CTBot.h>

Preferences preferences;
WebServer server(80);

String ssid = "";
String password = "";
String botToken = "";
int64_t chatID = 0;

CTBot miBot;
HardwareSerial Arduino(2);  // RX2 = 16, TX2 = 17

int64_t toInt64(String s) {
  return (int64_t)atoll(s.c_str());
}
// -------------------------------------------------------------
//   HTML PARA CONFIGURAR PARAMETROS DESDE EL NAVEGADOR
// -------------------------------------------------------------
String getConfigPage() {
  return String(R"(
    <!DOCTYPE html>
    <html>
      <body style='font-family: Arial;'>
        <h2>Configuración Alimentador</h2>
        <form action='/save' method='POST'>
          <label>WiFi SSID:</label><br>
          <input name='ssid'><br><br>

          <label>WiFi Password:</label><br>
          <input type='password' name='password'><br><br>

          <label>Telegram Bot Token:</label><br>
          <input name='bot'><br><br>

          <label>Telegram Chat ID:</label><br>
          <input name='chat'><br><br>

          <input type='submit' value='Guardar'>
        </form>
      </body>
    </html>
  )");
}

// -------------------------------------------------------------
//  GUARDAR CONFIGURACION EN MEMORIA (Preferences)
// -------------------------------------------------------------
void handleSave() {
  preferences.begin("config", false);

  preferences.putString("ssid", server.arg("ssid"));
  preferences.putString("pass", server.arg("password"));
  preferences.putString("bot", server.arg("bot"));
  preferences.putString("chat", server.arg("chat"));   // ✔ CORREGIDO

  preferences.end();

  Serial.print("ChatID guardado: ");
  Serial.println(toInt64(server.arg("chat")));  // para comprobar

  server.send(200, "text/html",
              "<h2>Datos guardados ✔</h2><p>Reinicia el ESP32.</p>");
}

// -------------------------------------------------------------
//  PORTAL DE CONFIGURACIÓN EN ACCESS POINT
// -------------------------------------------------------------
void startConfigPortal() {
  Serial.println("⚙ Iniciando portal de configuración...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("AlimentadorConfig", "12345678");  // contraseña opcional

  Serial.print("IP del portal: ");
  Serial.println(WiFi.softAPIP());  // normalmente 192.168.4.1

  server.on("/", []() {
    server.send(200, "text/html", getConfigPage());
  });

  server.on("/save", HTTP_POST, handleSave);

  server.begin();
}


// -------------------------------------------------------------
//                  SETUP
// -------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Arduino.begin(9600, SERIAL_8N1, 16, 17);

  preferences.begin("config", true);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("pass", "");
  botToken = preferences.getString("bot", "");
  String chatStr = preferences.getString("chat", "0");
  chatID = toInt64(chatStr);

  preferences.end();

  Serial.print("ChatID cargado desde memoria: ");
  Serial.println(chatID);

  // Si faltan datos → abrir modo configuración
  if (ssid == "" || password == "" || botToken == "" || chatID == 0) {
    startConfigPortal();
    return;
  }

  // Conexión WiFi UNA SOLA VEZ
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);

 WiFi.mode(WIFI_STA);
WiFi.begin(ssid.c_str(), password.c_str());

Serial.println("Conectando al WiFi...");

unsigned long inicioWifi = millis();
while (WiFi.status() != WL_CONNECTED) {
  delay(300);
  Serial.print(".");

  if (millis() - inicioWifi > 10000) {
    Serial.println("\nWiFi no disponible → Iniciando portal");
    
    ssid = "";
    password = "";
    botToken = "";
    chatID = 0;

    startConfigPortal();
    return;
  }
}

  Serial.println("\n✔ WiFi conectado");

  // Inicializar Telegram UNA VEZ
  miBot.setTelegramToken(botToken);

  if (miBot.testConnection()) {
    Serial.println("✔ Conexión Telegram OK");
    miBot.sendMessage(chatID, "🤖 Alimentador conectado\nUsa: ALIMENTAR o AGUA");
  } else {
    Serial.println("❌ Error Telegram");
  }
}


// -------------------------------------------------------------
//                  LOOP
// -------------------------------------------------------------
void loop() {
  // Si está en modo configuración, manejar webserver
  if (ssid == "" || password == "" || botToken == "" || chatID == 0) {
    server.handleClient();
    return;
  }

  TBMessage msg;

  // SOLO UNA VEZ
  if (miBot.getNewMessage(msg)) {

    Serial.print("Mensaje recibido de ID: ");
    Serial.println(msg.sender.id);

    Serial.print("Chat ID permitido: ");
    Serial.println(chatID);

    String texto = msg.text;
    texto.toUpperCase();

    if (msg.sender.id == chatID) {

      if (texto == "ALIMENTAR") {
        alimentar(msg.sender.id);
      }
      else if (texto == "AGUA") {
        servirAgua(msg.sender.id);
      }
      else {
        miBot.sendMessage(msg.sender.id,
                          "Comando no válido. Usa: ALIMENTAR o AGUA");
      }
    }
    else {
      miBot.sendMessage(msg.sender.id, "No tienes autorización.");
    }
  }

  delay(100);
}


// -------------------------------------------------------------
//              FUNCIONES ORIGINALES
// -------------------------------------------------------------
void alimentar(int64_t idChat) {
  miBot.sendMessage(idChat, "🍽 Verificando alimentador...");
  Arduino.println("ALIMENTAR");

  unsigned long inicio = millis();
  while (millis() - inicio < 15000) {
    if (Arduino.available()) {
      String respuesta = Arduino.readStringUntil('\n');
      respuesta.trim();
      Serial.println("Arduino -> " + respuesta);

      if (respuesta.indexOf("ALIMENTACION_COMPLETADA_CONFIRMADA - Comida llegó al plato") >= 0) {
        miBot.sendMessage(idChat, "✅ Comida servida correctamente");
        return;
      }
      if (respuesta.indexOf("❌ No hay comida en el depósito") >= 0) {
        miBot.sendMessage(idChat, "🚫 Sin comida en el depósito");
        return;
      }
      if (respuesta.indexOf("⚠ Motor activado, pero no se detectó comida en el plato") >= 0) {
        miBot.sendMessage(idChat, "Motor activado pero sin confirmacion de comida en el plato");
        return;
      }
      if (respuesta.indexOf("ALIMENTACION_COMPLETADA_CONFIRMADA - Comida ya presente") >= 0) {
        miBot.sendMessage(idChat, "✅ Comida ya presente en el plato");
        return;
      }
    }
    delay(50);
  }
  miBot.sendMessage(idChat, "⚠ Error: el alimentador no respondió a tiempo.");
}

void servirAgua(int64_t idChat) {
  miBot.sendMessage(idChat, "💧 Activando bomba de agua...");
  Arduino.println("AGUA");

  unsigned long inicio = millis();
  while (millis() - inicio < 15000) {
    if (Arduino.available()) {
      String respuesta = Arduino.readStringUntil('\n');
      respuesta.trim();
      Serial.println("Arduino -> " + respuesta);

      if (respuesta.indexOf("🚫 Sin agua en el almacén") >= 0) {
        miBot.sendMessage(idChat, "🚫 Sin agua en el almacén");
        return;
      }
      if (respuesta.indexOf("⚠ Bomba accionada pero el plato sigue vacío") >= 0) {
        miBot.sendMessage(idChat, "⚠ Bomba accionada, pero no hay confirmacion de en el plato");
        return;
      }
      if (respuesta.indexOf("💧 Agua cargada en el plato") >= 0) {
        miBot.sendMessage(idChat, "💧 El plato ya tenía agua");
        return;
      }
      if (respuesta.indexOf("✅ Agua servida y detectada en el plato") >= 0) {
        miBot.sendMessage(idChat, "✅ Agua servida y detectada en el plato");
        return;
      }
    }
    delay(50);
  }

  miBot.sendMessage(idChat, "⚠ Error: no hubo respuesta del sistema (Timeout).");
}