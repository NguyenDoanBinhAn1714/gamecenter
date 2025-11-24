#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "index.h"
#include "game_race.h"
#include "game_led.h"
#include "game_fight.h" // <-- Thêm file game mới

const char* ssid = "ESP32_Game_Center";
const char* password = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// --- GAME 1: ĐUA XE ---
#define MAX_RACE_PLAYERS 4
uint32_t race_players[MAX_RACE_PLAYERS] = {0};
int race_scores[MAX_RACE_PLAYERS] = {0};
bool race_ended = false;

// --- GAME 2: ĐỐI KHÁNG (FIGHT) ---
// 0:None, 1:Rock, 2:Paper, 3:Scissors
int fight_moves[2] = {0, 0}; 
int fight_hp[2] = {100, 100};
uint32_t fight_ids[2] = {0, 0};

// --- GAME 3: LED ---
#define LED_PIN 2

// -------------------------------------------------------------------------
// LOGIC GAME ĐỐI KHÁNG
// -------------------------------------------------------------------------
void broadcastFightState(String statusText) {
  // Gửi trạng thái: hp1,hp2,move1(text),move2(text),status
  String m1 = "none"; if(fight_moves[0]==1) m1="rock"; if(fight_moves[0]==2) m1="paper"; if(fight_moves[0]==3) m1="scissors";
  String m2 = "none"; if(fight_moves[1]==1) m2="rock"; if(fight_moves[1]==2) m2="paper"; if(fight_moves[1]==3) m2="scissors";
  
  // Nếu chưa lật bài thì giấu move đi
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
      resText = "P1 THẮNG HIỆP!";
      fight_hp[1] -= dmg;
    } else {
      resText = "P2 THẮNG HIỆP!";
      fight_hp[0] -= dmg;
    }

    // Kiểm tra hết máu
    if(fight_hp[0] <= 0) { fight_hp[0]=0; ws.textAll("fight:end:1"); } // P2 Win (index 1)
    else if(fight_hp[1] <= 0) { fight_hp[1]=0; ws.textAll("fight:end:0"); } // P1 Win (index 0)
    
    broadcastFightState(resText);

    // Reset move sau 2 giây (để người chơi kịp nhìn kết quả)
    // Ở đây ta chỉ reset biến move về 0, client cần cập nhật lại
    fight_moves[0] = 0;
    fight_moves[1] = 0;
    
    // Lưu ý: Trong môi trường thực tế cần timer non-blocking, nhưng để đơn giản ta giữ nguyên logic chờ vòng tiếp theo
  }
}

// -------------------------------------------------------------------------
// XỬ LÝ WEBSOCKET CHUNG
// -------------------------------------------------------------------------
void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_CONNECT) {
    // Xếp chỗ cho Game Fight (chỉ nhận 2 người)
    int fSlot = -1;
    if(fight_ids[0] == 0) { fight_ids[0] = client->id(); fSlot = 0; }
    else if(fight_ids[1] == 0) { fight_ids[1] = client->id(); fSlot = 1; }
    
    if(fSlot != -1) client->text("fight:welcome:" + String(fSlot));
    
    // Xếp chỗ cho Game Race (như cũ)
    int rSlot = -1;
    for(int i=0; i<MAX_RACE_PLAYERS; i++) if(race_players[i]==0) { race_players[i]=client->id(); rSlot=i; break; }
    if(rSlot != -1) client->text("welcome," + String(rSlot));

  } else if(type == WS_EVT_DISCONNECT) {
    // Xóa người chơi
    if(fight_ids[0] == client->id()) fight_ids[0] = 0;
    if(fight_ids[1] == client->id()) fight_ids[1] = 0;
    
    for(int i=0; i<MAX_RACE_PLAYERS; i++) if(race_players[i] == client->id()) { race_players[i]=0; race_scores[i]=0; }
  
  } else if(type == WS_EVT_DATA) {
    String msg = "";
    for(size_t i=0; i<len; i++) msg += (char)data[i];

    // --- GAME FIGHT LOGIC ---
    if(msg.startsWith("fight:")) {
      String cmd = msg.substring(6); // Bỏ chữ "fight:"
      int pIndex = (client->id() == fight_ids[0]) ? 0 : (client->id() == fight_ids[1]) ? 1 : -1;

      if(pIndex != -1) {
        if(cmd == "rock") fight_moves[pIndex] = 1;
        else if(cmd == "paper") fight_moves[pIndex] = 2;
        else if(cmd == "scissors") fight_moves[pIndex] = 3;
        else if(cmd == "reset") { fight_hp[0]=100; fight_hp[1]=100; fight_moves[0]=0; fight_moves[1]=0; broadcastFightState("Ván mới!"); }
        
        checkFightResult();
      }
    }
    
    // --- GAME RACE LOGIC ---
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

    // --- GAME LED ---
    else if(msg.startsWith("led:")) {
       if(msg == "led:on") digitalWrite(LED_PIN, HIGH);
       if(msg == "led:off") digitalWrite(LED_PIN, LOW);
    }
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){ req->send_P(200, "text/html", index_html); });
  server.on("/race", HTTP_GET, [](AsyncWebServerRequest *req){ req->send_P(200, "text/html", game_html); });
  server.on("/led", HTTP_GET, [](AsyncWebServerRequest *req){ req->send_P(200, "text/html", game_led_html); });
  
  // Route cho game mới
  server.on("/fight", HTTP_GET, [](AsyncWebServerRequest *req){ req->send_P(200, "text/html", game_fight_html); });

  server.begin();
}

void loop(){ ws.cleanupClients(); delay(1); }