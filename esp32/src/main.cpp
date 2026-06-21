#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "esp_camera.h"
#include <Wire.h>

const char* ssid = "@";
const char* password = "12345678900";

// Broches Ai-Thinker ESP32-S3-CAM
#define PWDN_GPIO_NUM      -1
#define RESET_GPIO_NUM     -1
#define XCLK_GPIO_NUM      15
#define SIOD_GPIO_NUM      4
#define SIOC_GPIO_NUM      5

#define Y9_GPIO_NUM        16
#define Y8_GPIO_NUM        17
#define Y7_GPIO_NUM        18
#define Y6_GPIO_NUM        12
#define Y5_GPIO_NUM        10
#define Y4_GPIO_NUM        8
#define Y3_GPIO_NUM        9
#define Y2_GPIO_NUM        11
#define VSYNC_GPIO_NUM      6
#define HREF_GPIO_NUM       7
#define PCLK_GPIO_NUM       13

AsyncWebServer server(80);

bool setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0   = Y2_GPIO_NUM;
  config.pin_d1   = Y3_GPIO_NUM;
  config.pin_d2   = Y4_GPIO_NUM;
  config.pin_d3   = Y5_GPIO_NUM;
  config.pin_d4   = Y6_GPIO_NUM;
  config.pin_d5   = Y7_GPIO_NUM;
  config.pin_d6   = Y8_GPIO_NUM;
  config.pin_d7   = Y9_GPIO_NUM;
  config.pin_xclk    = XCLK_GPIO_NUM;
  config.pin_pclk    = PCLK_GPIO_NUM;
  config.pin_vsync   = VSYNC_GPIO_NUM;
  config.pin_href    = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn   = PWDN_GPIO_NUM;
  config.pin_reset  = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format  = PIXFORMAT_JPEG;
  config.frame_size    = FRAMESIZE_CIF;
  config.jpeg_quality  = 12;
  config.fb_count      = 2;
  config.grab_mode     = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location   = CAMERA_FB_IN_PSRAM;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x\n", err);
    return false;
  }
  Serial.println("Camera ready.");
  return true;
}

// Envoie une réponse multipart pour un flux MJPEG
void handleStream(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response = request->beginChunkedResponse(
    "multipart/x-mixed-replace; boundary=frame",
    [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
      static camera_fb_t *fb = NULL;
      static size_t pos = 0;
      static uint8_t headerSent = false;

      if (index == 0) {
        // Début du flux : on récupère une première frame
        fb = esp_camera_fb_get();
        if (!fb || fb->format != PIXFORMAT_JPEG) {
          if (fb) esp_camera_fb_return(fb);
          fb = NULL;
          return 0;
        }
        pos = 0;
        headerSent = false;
      }

      if (!fb) return 0;

      // Tant qu'on n'a pas envoyé l'en-tête multipart pour cette frame
      if (!headerSent) {
        // Construire l'en-tête
        char partHeader[128];
        int hdrLen = snprintf(partHeader, sizeof(partHeader),
                              "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                              fb->len);
        // Envoyer l'en-tête (ou le reste de l'en-tête) d'abord
        // Comme c'est une réponse chunked, on peut renvoyer l'en-tête puis l'image
        // Mais on ne peut renvoyer qu'un seul bloc à la fois (ce que permet le buffer)
        // On va donc copier progressivement
        // Pour simplifier, on pourrait envoyer tout l'en-tête d'un coup si maxLen est assez grand
        // Sinon, on fragmente.
        if (hdrLen <= maxLen) {
          memcpy(buffer, partHeader, hdrLen);
          headerSent = true;
          return hdrLen;
        } else {
          // Si le buffer est trop petit, on risque des problèmes.
          // On peut tronquer ou gérer un état. On part du principe que maxLen > 128.
          return 0;
        }
      }

      // Envoi des données JPEG
      size_t remain = fb->len - pos;
      size_t toSend = (remain < maxLen) ? remain : maxLen;
      memcpy(buffer, fb->buf + pos, toSend);
      pos += toSend;

      // Si on a fini d'envoyer cette frame, on prépare la suivante
      if (pos >= fb->len) {
        esp_camera_fb_return(fb);
        fb = esp_camera_fb_get();
        if (!fb || fb->format != PIXFORMAT_JPEG) {
          if (fb) esp_camera_fb_return(fb);
          fb = NULL;
          return 0;
        }
        pos = 0;
        headerSent = false; // on enverra le nouvel en-tête
      }

      return toSend;
    }
  );

  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Cache-Control", "no-cache");
  response->addHeader("Content-Disposition", "inline; filename=stream.jpg");
  request->send(response);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connexion WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté !");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!setupCamera()) {
    Serial.println("Camera fail");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head><title>ESP32-S3 Stream</title></head>
      <body>
        <h1>ESP32-S3 Camera</h1>
        <img src="/stream" style="width:100%; max-width:640px;">
      </body>
      </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  server.on("/stream", HTTP_GET, handleStream);

  server.begin();
  Serial.print("Serveur demarrer sur : ");
  Serial.println("10.248.17.53");
}

void loop() {
  delay(10);
}