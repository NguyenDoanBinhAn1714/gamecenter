#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// --- 1. NHÚNG CÁC FILE GIAO DIỆN (HEADER FILES) ---
// Đảm bảo bạn đã tạo đủ các Tab này trong Arduino IDE
#include "index.h"        // Menu Trang chủ
#include "game_race.h"    // Game Đua xe
#include "game_fight.h"   // Game Đối kháng
#include "game_reflex.h"  // Game Phản xạ
#include "game_led.h"     // App Điều khiển LED
#include "game_maze.h"    // Game Mê cung ma (Mới)

// --- 2. CẤU HÌNH WIFI ---
const char* ssid = "ESP32_Game_Center"; // Tên Wifi phát ra
const char* password = "12345678";      // Mật khẩu

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ============================================================
//   KHAI BÁO BIẾN TOÀN CỤC CHO TỪNG GAME
// ============================================================

// --- GAME 1: ĐUA XE (RACE) ---
#define MAX_RACE_PLAYERS 4
uint32_t race_players[MAX_RACE_PLAYERS] = {0};
int race_scores[MAX_RACE_PLAYERS] = {0};
bool race_ended = false;

// --- GAME 2: ĐỐI KHÁNG (FIGHT) ---
// 0:None, 1:Rock, 2:Paper, 3:Scissors
int fight_moves[2] = {0, 0}; 
int fight_hp[2] = {100, 100};
uint32_t fight_ids[2] = {0, 0};

// --- GAME 3: PHẢN XẠ (REFLEX) ---
uint32_t reflex_ids[4] = {0};
int reflex_scores[4] = {0};
int target_color = -1;          // -1: Chờ, 0-3: Màu mục tiêu
unsigned long reflex_timer = 0; // Bộ đếm thời gian
bool reflex_round_active = false;
#define WIN_REFLEX_SCORE 5      // Điểm thắng

// --- GAME 4: LED ---
#define LED_PIN 2               // Chân đèn LED trên mạch

// --- GAME 5: MAZE ---
// Game này xử lý 100% trên trình duyệt (Client-side)
// Server chỉ cần phục vụ file HTML, không cần xử lý logic WebSocket.

// ============================================================
//   CÁC HÀM HỖ TRỢ LOGIC (HELPER FUNCTIONS)
// ============================================================

// --- LOGIC FIGHT ---
void broadcastFightState(String statusText) {
  String m1 = "none"; if(fight_moves[0]==1) m1="rock"; if(fight_moves[0]==2) m1="paper"; if(fight_moves[0]==3) m1="scissors";
  String m2 = "none"; if(fight_moves[1]==1) m2="rock"; if(fight_moves[1]==2) m2="paper"; if(fight_moves[1]==3) m2="scissors";
  
  // Nếu chưa lật bài (cả 2 chưa chọn xong) thì giấu move đi
  bool reveal = (fight_moves[0] != 0 && fight_moves[1] != 0);
  if(!reveal) { m1 = "none"; m2 = "none"; }

  String msg = "fight:update:" + String(fight_hp[0]) + "," + String(fight_hp[1]) + "," + m1 + "," + m2 + "," + (reveal ? statusText : "null");
  ws.textAll(msg);
}

void checkFightResult() {
  if (fight_moves[0] != 0 && fight_moves[1] != 0) {
    int p1 = fight_moves[0];
    int p2 = fight_moves[1];
    String resText = "";
    int dmg = 20; // Sát thương mỗi hiệp

    if (p1 == p2) {
      resText = "HÒA!";
    } else if ((p1 == 1 && p2 == 3) || (p1 == 2 && p2 == 1) || (p1 == 3 && p2 == 2)) {
      resText = "P1 TRÚNG!";
      fight_hp[1] -= dmg;
    } else {
      resText = "P2 TRÚNG!";
      fight_hp[0] -= dmg;
    }

    // Kiểm tra hết máu
    if(fight_hp[0] <= 0) { fight_hp[0]=0; ws.textAll("fight:end:1"); } // P2 Win (index 1)
    else if(fight_hp[1] <= 0) { fight_hp[1]=0; ws.textAll("fight:end:0"); } // P1 Win (index 0)
    
    broadcastFightState(resText);

    // Reset move
    fight_moves[0] = 0;
    fight_moves[1] = 0;
  }
}

// --- LOGIC REFLEX ---
void broadcastReflexScores() {
  String msg = "reflex:scores:";
  for(int i=0; i<4; i++) msg += String(reflex_scores[i]) + (i<3?",":"");
  ws.textAll(msg);
}

void startReflexRound() {
  target_color = random(0, 4); // Chọn màu ngẫu nhiên 0-3
  reflex_round_active = true;
  ws.textAll("reflex:show:" + String(target_color));
}

void resetReflexGame() {
  for(int i=0; i<4; i++) reflex_scores[i] = 0;
  reflex_round_active = false;
  reflex_timer = millis() + 3000; // Chờ 3s rồi bắt đầu round mới
  ws.textAll("reflex:wait");
  broadcastReflexScores();
}

// ============================================================
//   XỬ LÝ SỰ KIỆN WEBSOCKET (REALTIME)
// ============================================================
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_CONNECT) {
    // 1. Xếp chỗ Reflex
    int rfSlot = -1;
    for(int i=0; i<4; i++) if(reflex_ids[i]==0) { reflex_ids[i]=client->id(); rfSlot=i; break; }
    if(rfSlot != -1) client->text("reflex:welcome:" + String(rfSlot));

    // 2. Xếp chỗ Fight
    int fSlot = -1;
    if(fight_ids[0] == 0) { fight_ids[0] = client->id(); fSlot = 0; }
    else if(fight_ids[1] == 0) { fight_ids[1] = client->id(); fSlot = 1; }
    if(fSlot != -1) client->text("fight:welcome:" + String(fSlot));
    
    // 3. Xếp chỗ Race
    int rSlot = -1;
    for(int i=0; i<MAX_RACE_PLAYERS; i++) if(race_players[i]==0) { race_players[i]=client->id(); rSlot=i; break; }
    if(rSlot != -1) client->text("welcome," + String(rSlot));

  } else if(type == WS_EVT_DISCONNECT) {
    // Xóa người chơi khi mất kết nối
    for(int i=0; i<4; i++) if(reflex_ids[i] == client->id()) { reflex_ids[i]=0; reflex_scores[i]=0; }
    if(fight_ids[0] == client->id()) fight_ids[0] = 0;
    if(fight_ids[1] == client->id()) fight_ids[1] = 0;
    for(int i=0; i<MAX_RACE_PLAYERS; i++) if(race_players[i] == client->id()) { race_players[i]=0; race_scores[i]=0; }
  
  } else if(type == WS_EVT_DATA) {
    String msg = "";
    for(size_t i=0; i<len; i++) msg += (char)data[i];

    // --- XỬ LÝ GAME REFLEX ---
    if(msg.startsWith("reflex:")) {
      String cmd = msg.substring(7);
      if(cmd == "reset") { resetReflexGame(); }
      else {
        // Người chơi bấm màu: "reflex:2"
        if(reflex_round_active) {
          int playerChoice = cmd.toInt();
          int pIdx = -1;
          for(int i=0; i<4; i++) if(reflex_ids[i] == client->id()) pIdx=i;

          if(pIdx != -1) {
            if(playerChoice == target_color) {
              // ĐÚNG & NHANH NHẤT -> Cộng điểm
              reflex_scores[pIdx]++;
              reflex_round_active = false;
              reflex_timer = millis() + random(2000, 5000); // Random 2-5s
              ws.textAll("reflex:wait");
              broadcastReflexScores();
              
              if(reflex_scores[pIdx] >= WIN_REFLEX_SCORE) ws.textAll("reflex:win:" + String(pIdx));

            } else {
              // SAI -> Trừ điểm
              reflex_scores[pIdx]--;
              broadcastReflexScores();
            }
          }
        }
      }
    }

    // --- XỬ LÝ GAME FIGHT ---
    else if(msg.startsWith("fight:")) {
      String cmd = msg.substring(6);
      int pIndex = (client->id() == fight_ids[0]) ? 0 : (client->id() == fight_ids[1]) ? 1 : -1;

      if(pIndex != -1) {
        if(cmd == "rock") fight_moves[pIndex] = 1;
        else if(cmd == "paper") fight_moves[pIndex] = 2;
        else if(cmd == "scissors") fight_moves[pIndex] = 3;
        else if(cmd == "reset") { fight_hp[0]=100; fight_hp[1]=100; fight_moves[0]=0; fight_moves[1]=0; broadcastFightState("Ván mới!"); }
        
        checkFightResult();
      }
    }
    
    // --- XỬ LÝ GAME RACE ---
    else if(msg == "tap" && !race_ended) {
       for(int i=0; i<MAX_RACE_PLAYERS; i++) {
        if(race_players[i] == client->id()) {
          race_scores[i]++;
          if(race_scores[i] >= 100) {
            race_ended = true;
            ws.textAll("win," + String(i));
          } else {
            String updateMsg = "update";
            for(int j=0; j<MAX_RACE_PLAYERS; j++) updateMsg += "," + String(race_scores[j]);
            ws.textAll(updateMsg);
          }
          break;
        }
      }
    }
    else if(msg == "reset") { // Reset game race
       for(int i=0; i<MAX_RACE_PLAYERS; i++) race_scores[i]=0;
       race_ended = false;
       ws.textAll("update,0,0,0,0");
    }

    // --- XỬ LÝ LED ---
    else if(msg.startsWith("led:")) {
       if(msg == "led:on") digitalWrite(LED_PIN, HIGH);
       if(msg == "led:off") digitalWrite(LED_PIN, LOW);
    }
  }
}

// ============================================================
//   SETUP & LOOP
// ============================================================
void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Phát Wifi
  WiFi.softAP(ssid, password);
  Serial.println("\n--- SERVER STARTED ---");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // WebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // --- ĐỊNH TUYẾN (ROUTER) ---
  // Điều hướng yêu cầu từ trình duyệt đến đúng file giao diện
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", index_html); 
  });
  
  server.on("/race", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", game_html); // Biến chứa HTML của game Race
  });

  server.on("/fight", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", game_fight_html); 
  });

  server.on("/reflex", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", game_reflex_html); 
  });

  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", game_led_html); 
  });

  // GAME MỚI: MÊ CUNG
  server.on("/maze", HTTP_GET, [](AsyncWebServerRequest *req){ 
    req->send_P(200, "text/html", game_maze_html); 
  });

  server.begin();
  
  // Khởi động timer cho game reflex
  reflex_timer = millis() + 3000;
}

void loop(){
  ws.cleanupClients();
  
  // Loop riêng cho Game Phản Xạ: Tự động bật màu sau thời gian ngẫu nhiên
  if(!reflex_round_active && millis() > reflex_timer && reflex_timer > 0) {
    startReflexRound();
    reflex_timer = 0; // Dừng timer chờ người bấm
  }
  
  delay(1);
}