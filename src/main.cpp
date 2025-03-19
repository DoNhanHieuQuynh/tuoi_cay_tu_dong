#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <HardwareSerial.h>
#include "config.h"

// Cấu hình WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Khởi tạo Web Server, NTP Client và UART
WebServer server(80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // GMT+7 (Việt Nam)
HardwareSerial MySerial(2); // UART2: RX=16, TX=17 trên ESP32

// Biến toàn cục
String uartData = "";        // Dữ liệu nhận từ UART
String ledState = "OFF";     // Trạng thái máy bơm
String temperature = "N/A";  // Nhiệt độ
String humidity = "N/A";     // Độ ẩm
int onHour = -1, onMinute = -1;    // Thời gian bật tự động
int offHour = -1, offMinute = -1;  // Thời gian tắt tự động
int prevHour = -1, prevMinute = -1; // Thời gian trước đó để tránh lặp lệnh

// Hàm xử lý trang chủ (index.html)
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

// Hàm xử lý file CSS (style.css)
void handleStyle() {
  File file = SPIFFS.open("/style.css", "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, "text/css");
  file.close();
}

// Hàm khởi tạo hệ thống
void setup() {
  Serial.begin(115200); // Khởi động Serial để debug
  MySerial.begin(115200, SERIAL_8N1, 16, 17); // UART2 cho giao tiếp với STM32

  // Khởi tạo SPIFFS để lưu trữ file HTML và CSS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
    return;
  }

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());

  // Khởi tạo NTP để lấy thời gian thực
  timeClient.begin();
  timeClient.update();

  // Thiết lập các endpoint cho Web Server
  server.on("/", handleRoot); // Trang chủ
  server.on("/style.css", handleStyle); // File CSS
  server.on("/state", []() { server.send(200, "text/plain", ledState); }); // Trạng thái máy bơm
  server.on("/temperature", []() { server.send(200, "text/plain", temperature); }); // Nhiệt độ
  server.on("/humidity", []() { server.send(200, "text/plain", humidity); }); // Độ ẩm
  server.on("/time", []() { // Thời gian hiện tại
    String timeStr = String(timeClient.getHours()) + ":" + 
                     String(timeClient.getMinutes()) + ":" + 
                     String(timeClient.getSeconds());
    server.send(200, "text/plain", timeStr);
  });
  server.on("/on", []() { // Bật máy bơm
    MySerial.print('1');
    server.send(200, "text/plain", "ON");
  });
  server.on("/off", []() { // Tắt máy bơm
    MySerial.print('0');
    server.send(200, "text/plain", "OFF");
  });
  server.on("/setAlarm", []() { // Cài đặt hẹn giờ
    if (server.hasArg("onHour") && server.hasArg("onMinute") && 
        server.hasArg("offHour") && server.hasArg("offMinute")) {
      onHour = server.arg("onHour").toInt();
      onMinute = server.arg("onMinute").toInt();
      offHour = server.arg("offHour").toInt();
      offMinute = server.arg("offMinute").toInt();
      server.send(200, "text/plain", "Alarm set successfully");
    } else {
      server.send(400, "text/plain", "Missing parameters");
    }
  });

  server.begin();
  Serial.println("Web server started");
}

// Hàm vòng lặp chính
void loop() {
  server.handleClient(); // Xử lý các yêu cầu từ web
  timeClient.update();  // Cập nhật thời gian từ NTP

  // Xử lý dữ liệu từ UART (nhận từ STM32)
  while (MySerial.available()) {
    char c = MySerial.read();
    if (c == '\r') { // Khi nhận được ký tự xuống dòng, phân tích dữ liệu
      int state;
      float temp, humi;
      if (sscanf(uartData.c_str(), "STATE:%d TEMP:%f HUMI:%f", &state, &temp, &humi) == 3) {
        ledState = (state == 1) ? "ON" : "OFF";
        temperature = String(temp, 1); // 1 chữ số thập phân
        humidity = String(humi, 1);    // 1 chữ số thập phân
      }
      uartData = ""; // Reset chuỗi dữ liệu
    } else if (c >= 32 && c <= 126) { // Chỉ thêm các ký tự in được
      uartData += c;
    }
  }

  // Kiểm tra và thực hiện hẹn giờ tự động
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  // Bật máy bơm nếu đến giờ cài đặt
  if (onHour != -1 && currentHour == onHour && currentMinute == onMinute && 
      (prevHour != currentHour || prevMinute != currentMinute)) {
    MySerial.print('1');
  }
  // Tắt máy bơm nếu đến giờ cài đặt
  if (offHour != -1 && currentHour == offHour && currentMinute == offMinute && 
      (prevHour != currentHour || prevMinute != currentMinute)) {
    MySerial.print('0');
  }
  // Cập nhật thời gian trước đó để tránh lặp lệnh
  prevHour = currentHour;
  prevMinute = currentMinute;
}