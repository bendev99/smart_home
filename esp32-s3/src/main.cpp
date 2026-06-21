#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ========== CONFIGURATION WiFi ==========
const char* ssid = "@";
const char* password = "12345678900";

// ========== CONFIGURATION MQTT ==========
const char* mqtt_server = "10.248.17.37"; //Adresse IP du PC
const int   mqtt_port   = 1883;
const char* mqtt_client_id = "ESP32S3-LED";

// Topics
const char* topic_cmd    = "esp32/led/cmd";
const char* topic_state  = "esp32/led/state";

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ========== LED ==========
// La plupart des ESP32-S3 utilisent GPIO 48 pour la LED_BUILTIN
#ifndef LED_BUILTIN
  #define LED_BUILTIN 48
#endif

// ========== FONCTIONS MQTT ==========
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message reçu [");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.println(message);

  if (message == "led_on") {
    digitalWrite(LED_BUILTIN, HIGH);
    mqtt.publish(topic_state, "on");
    Serial.println("LED allumée");
  }
  else if (message == "led_off") {
    digitalWrite(LED_BUILTIN, LOW);
    mqtt.publish(topic_state, "off");
    Serial.println("LED éteinte");
  }
  else {
    Serial.println("Commande inconnue");
  }
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println(" connecté");
      mqtt.subscribe(topic_cmd);
    } else {
      Serial.print("échec, code = ");
      Serial.print(mqtt.state());
      Serial.println(" nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println();
  Serial.print("Connexion au WiFi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
  Serial.print("Adresse IP ESP32 : ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(callback);
  reconnectMQTT();

  mqtt.publish(topic_state, "off");
}

// ========== LOOP ==========
void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();
  delay(10);
}