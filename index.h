const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 Game Center</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #181818; color: white; margin: 0; padding: 20px; text-align: center; }
    h1 { color: #f1c40f; text-transform: uppercase; margin-bottom: 30px; letter-spacing: 2px; }
    
    /* Lưới hiển thị game */
    .game-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 20px;
      max-width: 800px;
      margin: 0 auto;
    }

    /* Thẻ Game (Card) */
    .game-card {
      background: #252525;
      border-radius: 15px;
      padding: 20px;
      border: 1px solid #333;
      transition: transform 0.2s, box-shadow 0.2s;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    .game-card:hover { transform: translateY(-5px); box-shadow: 0 10px 20px rgba(0,0,0,0.5); border-color: #f1c40f; }

    .icon { font-size: 50px; margin-bottom: 10px; }
    h3 { margin: 10px 0; color: #fff; }
    p { color: #aaa; font-size: 14px; margin-bottom: 20px; height: 40px; }

    .btn-play {
      background: linear-gradient(45deg, #3498db, #2980b9);
      color: white; text-decoration: none;
      padding: 10px 30px; border-radius: 25px;
      font-weight: bold; width: 80%;
    }
    .btn-play.race { background: linear-gradient(45deg, #e74c3c, #c0392b); }
    .btn-play.led { background: linear-gradient(45deg, #2ecc71, #27ae60); }
    
    .footer { margin-top: 40px; color: #555; font-size: 12px; }
  </style>
</head>
<body>

  <h1>🎮 ESP32 ARCADE 🎮</h1>

  <div class="game-grid">
    
    <div class="game-card">
      <div class="icon">🏎️</div>
      <h3>ĐUA TỐC ĐỘ</h3>
      <p>Game bấm nút loạn xạ cho 4 người chơi.</p>
      <a href="/race" class="btn-play race">CHƠI NGAY</a>
    </div>

    <div class="game-card">
      <div class="icon">💡</div>
      <h3>ĐIỀU KHIỂN LED</h3>
      <p>Bật tắt đèn LED trên ESP32 qua Wifi.</p>
      <a href="/led" class="btn-play led">CHƠI NGAY</a>
    </div>

    <div class="game-card">
      <div class="icon">⚔️</div>
      <h3>ĐẠI CHIẾN</h3>
      <p>Kéo - Búa - Bao (2 người).</p>
      <a href="/fight" class="btn-play" style="background: linear-gradient(45deg, #8e44ad, #9b59b6);">CHƠI NGAY</a>
    </div>

    <div class="game-card" style="opacity: 0.5;">
      <div class="icon">🔒</div>
      <h3>SẮP RA MẮT</h3>
      <p>Trò chơi bí mật đang phát triển...</p>
      <button disabled class="btn-play" style="background:#555; cursor: not-allowed;">Coming Soon</button>
    </div>

  </div>

  <div class="footer">System v2.0 - MultiGame Support</div>

</body>
</html>
)rawliteral";