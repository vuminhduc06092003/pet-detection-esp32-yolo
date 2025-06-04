#include "esp_camera.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <HTTPClient.h>

//------ cấu hình Wi-Fi & server -----------------
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char *ssid     = "WIFI GIANG VIEN";
const char *password = "dhdn7799";
const char *serverUrl = "http://172.16.71.135:5000/upload";   // Flask server

WebServer server(80);
//------------------------------------------------

//=========== Khởi tạo camera ===========
void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;   config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;   config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;   config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;   config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size   = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("❌ Camera init failed 0x%x\n", err);
  }
}

//=========== Gửi ảnh về Flask ===========
bool sendPhoto() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { Serial.println("❌ Capture failed"); return false; }

  WiFiClient client; HTTPClient http;
  http.begin(client, serverUrl);
  http.addHeader("Content-Type", "image/jpeg");
  int code = http.POST(fb->buf, fb->len);
  esp_camera_fb_return(fb);
  http.end();

  Serial.printf("HTTP Response: %d\n", code);
  return code == 200;
}

//=========== HTML trang chủ =============
void handleRoot() {
  String html = R"(
    <html><head><meta charset="UTF-8"></head><body>
      <h2>ESP32-CAM Demo</h2>
      <p><a href="/stream">Xem live-stream</a></p>
      <form action="/capture" method="POST">
        <button type="submit">Capture &amp; Gửi Ảnh</button>
      </form>
    </body></html>)";
  server.send(200, "text/html", html);
}

//=========== API /capture ===============
void handleCapture() {
  Serial.println("Da nhan yeu cau Capture!");
  if (sendPhoto()) {
    server.send(200, "text/plain", "Da gui anh thanh cong!");
    Serial.println("Da gui anh thanh cong!");
  } else {
    server.send(500, "text/plain", "Loi gui anh!");
    Serial.println("Loi gui anh!");
  }
}


//=========== API /stream (MJPEG) ========
void streamMjpeg() {
  WiFiClient client = server.client();

  camera_fb_t * fb = NULL;
  String resp = "HTTP/1.1 200 OK\r\n"
                "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(resp);

  while (client.connected()) {
    fb = esp_camera_fb_get();
    if (!fb) continue;

    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                   fb->len);
    client.write(fb->buf, fb->len);
    client.println("\r\n");
    esp_camera_fb_return(fb);

    if (!client.connected()) break;
    delay(30); // ~30 fps
  }
}

//=========== Setup ======================
void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) { Serial.print('.'); delay(500); }
  Serial.printf("\nWiFi OK, IP: %s\n", WiFi.localIP().toString().c_str());

  startCamera();

  server.on("/",               handleRoot);
  server.on("/capture", HTTP_POST, handleCapture);
  server.on("/stream", HTTP_GET, [](){ streamMjpeg(); });
  server.begin();
  Serial.println("Web server started ➜ / (root), /capture, /stream");
}

//=========== Loop =======================
void loop() {
  server.handleClient();
}
