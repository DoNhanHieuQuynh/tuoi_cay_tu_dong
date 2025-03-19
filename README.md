# Hệ Thống Điều Khiển Máy Bơm Thông Minh

Dự án này là một hệ thống nhúng sử dụng ESP32 để điều khiển máy bơm thông qua giao diện web. Hệ thống có các tính năng:

- Điều khiển máy bơm ON/OFF thông qua web interface
- Hiển thị nhiệt độ và độ ẩm môi trường
- Hẹn giờ tự động bật/tắt máy bơm
- Giao tiếp với STM32 thông qua UART
- Hiển thị thời gian thực từ NTP server

## Cấu hình Phần Cứng

- ESP32
- Cảm biến nhiệt độ và độ ẩm
- Mạch điều khiển máy bơm (STM32)
- Kết nối WiFi

## Cài Đặt

1. Cài đặt Arduino IDE
2. Cài đặt các thư viện cần thiết:
   - WiFi.h
   - WebServer.h
   - SPIFFS.h
   - NTPClient.h
   - WiFiUdp.h
   - HardwareSerial.h

3. Cấu hình WiFi trong file `src/main.cpp`:
   ```cpp
   const char* ssid = "Tên_WiFi";
   const char* password = "Mật_Khẩu_WiFi";
   ```

4. Upload code lên ESP32

## Sử Dụng

1. Kết nối ESP32 với nguồn điện
2. Truy cập địa chỉ IP của ESP32 trên trình duyệt web
3. Sử dụng giao diện web để:
   - Điều khiển máy bơm
   - Xem nhiệt độ và độ ẩm
   - Cài đặt hẹn giờ tự động

## Cấu Trúc Dự Án

```
├── src/
│   └── main.cpp
├── data/
│   ├── index.html
│   └── style.css
└── README.md
``` 