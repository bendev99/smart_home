#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

// ========== CONFIGURATION WiFi ==========
const char* ssid = "@";
const char* password = "12345678900";

// ========== CONFIGURATION MQTT ==========
const char* mqtt_server = "10.248.17.37";  // IP de votre PC
const int   mqtt_port   = 1883;
const char* mqtt_client_id = "ESP32S3-IoT";

// Topics
const char* topic_led_cmd = "esp32/led/cmd";
const char* topic_led_state = "esp32/led/state";
const char* topic_dht = "esp32/dht";         // température/humidité
const char* topic_distance = "esp32/distance"; // ultrason

WiFiClient espClient;
PubSubClient mqtt(espClient);

// ========== LED ==========
#ifndef LED_BUILTIN
  #define LED_BUILTIN 48
#endif

// ========== DHT22 ==========
#define DHTPIN 1
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0.0;
float humidity = 0.0;
unsigned long lastDHTRead = 0;

// ========== HC-SR04 ==========
#define TRIG_PIN 2
#define ECHO_PIN 3
float distance = 0.0;
unsigned long lastSonarRead = 0;

// ========== Fonctions ==========
void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
  }
}

void readSonar() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30 ms
  if (duration > 0) {
    distance = duration * 0.034 / 2; // cm
  } else {
    distance = -1; // erreur
  }
}

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
    mqtt.publish(topic_led_state, "on");
  }
  else if (message == "led_off") {
    digitalWrite(LED_BUILTIN, LOW);
    mqtt.publish(topic_led_state, "off");
  }
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Connexion MQTT...");
    if (mqtt.connect(mqtt_client_id)) {
      Serial.println(" connecté");
      mqtt.subscribe(topic_led_cmd);
    } else {
      Serial.print("échec, rc=");
      Serial.print(mqtt.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté, IP: " + WiFi.localIP().toString());

  mqtt.setServer(mqtt_server, mqtt_port);
  mqtt.setCallback(callback);
  reconnectMQTT();

  mqtt.publish(topic_led_state, "off");
}

void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();

  // Lecture DHT22 toutes les 5 secondes
  if (millis() - lastDHTRead > 5000) {
    lastDHTRead = millis();
    readDHT();
    String payload = "{\"temp\":" + String(temperature, 1) + ",\"hum\":" + String(humidity, 1) + "}";
    mqtt.publish(topic_dht, payload.c_str());
    Serial.println("DHT publié: " + payload);
  }

  // Lecture HC-SR04 toutes les 2 secondes
  if (millis() - lastSonarRead > 2000) {
    lastSonarRead = millis();
    readSonar();
    String payload = String(distance, 1);
    mqtt.publish(topic_distance, payload.c_str());
    Serial.println("Distance publiée: " + payload + " cm");
  }

  delay(10);
}