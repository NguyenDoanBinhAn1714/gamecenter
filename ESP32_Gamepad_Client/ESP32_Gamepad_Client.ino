#include <WiFi.h>
#include <WebSocketsClient.h>

// --- CẤU HÌNH ---
const char* ssid = "ESP32_Game_Center"; // Tên Wifi của con Server phát ra
const char* password = "12345678";
const char* server_host = "192.168.4.1"; // IP mặc định của Server
const int server_port = 80;

// Khai báo chân nút bấm
const int BTN_1 = 13; // Đỏ
const int BTN_2 = 12; // Xanh lá
const int BTN_3 = 14; // Xanh dương
const int BTN_4 = 27; // Vàng

WebSocketsClient webSocket;

// Biến chống nhiễu nút (Debounce)
unsigned long lastPress = 0;
const int debounceTime = 100;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.println("[GP] Đã kết nối Server!");
      // Gửi tin nhắn báo danh
      webSocket.sendTXT("IAM_GAMEPAD"); 
      break;
    case WStype_DISCONNECTED:
      Serial.println("[GP] Mất kết nối!");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Cài đặt chân nút bấm là INPUT_PULLUP (Không cần điện trở ngoài)
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);

  // Kết nối Wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println(" Connected!");

  // Kết nối WebSocket
  webSocket.begin(server_host, server_port, "/ws");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
}

void loop() {
  webSocket.loop();

  // Đọc trạng thái nút (Chống rung phím đơn giản)
  if (millis() - lastPress > debounceTime) {
    
    // Nút 1: Đua xe (Tap), Đối kháng (Búa/Rock), Phản xạ (Màu 0)
    if (digitalRead(BTN_1) == LOW) {
      webSocket.sendTXT("pad:0"); // Gửi lệnh mã hóa "pad:0"
      lastPress = millis();
    }
    
    // Nút 2: Đối kháng (Bao/Paper), Phản xạ (Màu 1)
    else if (digitalRead(BTN_2) == LOW) {
      webSocket.sendTXT("pad:1");
      lastPress = millis();
    }

    // Nút 3: Đối kháng (Kéo/Scissors), Phản xạ (Màu 2)
    else if (digitalRead(BTN_3) == LOW) {
      webSocket.sendTXT("pad:2");
      lastPress = millis();
    }

    // Nút 4: Phản xạ (Màu 3), Reset (nếu giữ lâu - tính năng nâng cao sau)
    else if (digitalRead(BTN_4) == LOW) {
      webSocket.sendTXT("pad:3");
      lastPress = millis();
    }
  }
}