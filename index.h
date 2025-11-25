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
      transition: filter 0.2s;
    }
    .btn-play:hover { filter: brightness(1.1); }

    /* Màu sắc riêng cho từng game */
    .race { background: linear-gradient(45deg, #e74c3c, #c0392b); }
    .fight { background: linear-gradient(45deg, #8e44ad, #9b59b6); }
    .reflex { background: linear-gradient(45deg, #00b894, #00cec9); }
    .led { background: linear-gradient(45deg, #f1c40f, #f39c12); color: #222; }
    
    .footer { margin-top: 40px; color: #555; font-size: 12px; }
  </style>
</head>
<body>

  <h1>🎮 ESP32 ARCADE 🎮</h1>

  <div class="game-grid">
    
    <div class="game-card">
      <div class="icon">🏎️</div>
      <h3>ĐUA TỐC ĐỘ</h3>
      <p>Bấm nút loạn xạ (Max 4 người).</p>
      <a href="/race" class="btn-play race">CHƠI NGAY</a>
    </div>

    <div class="game-card">
      <div class="icon">⚔️</div>
      <h3>ĐẠI CHIẾN</h3>
      <p>Kéo - Búa - Bao chiến thuật (2 người).</p>
      <a href="/fight" class="btn-play fight">CHƠI NGAY</a>
    </div>

    <div class="game-card">
      <div class="icon">⚡</div>
      <h3>VUA PHẢN XẠ</h3>
      <p>Thử thách độ nhanh tay lẹ mắt.</p>
      <a href="/reflex" class="btn-play reflex">CHƠI NGAY</a>
    </div>

    <div class="game-card">
      <div class="icon">💡</div>
      <h3>ĐIỀU KHIỂN LED</h3>
      <p>Test phần cứng, bật tắt đèn LED.</p>
      <a href="/led" class="btn-play led">MỞ APP</a>
    </div>

    <div class="game-card">
      <div class="icon">O_O</div>
      <h3>MÊ CUNG </h3>
      <p>Cẩn thận cái kết...</p>
      <a href="/maze" class="btn-play" style="background: linear-gradient(45deg, #000000, #434343);">THỬ GAN</a>
    </div>
  </div>

  <div class="footer">System v3.0 - Reflex Game Added</div>

</body>
</html>
)rawliteral";