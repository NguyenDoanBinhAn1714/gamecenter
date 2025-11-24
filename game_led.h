const char game_led_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Controller</title>
  <style>
    body { background: #111; color: white; text-align: center; font-family: sans-serif; padding-top: 50px; }
    .btn { padding: 20px 40px; font-size: 24px; border-radius: 10px; border: none; cursor: pointer; margin: 20px; }
    .on { background: #2ecc71; color: white; }
    .off { background: #e74c3c; color: white; }
    .home { display: block; margin-top: 50px; color: #3498db; text-decoration: none; font-size: 18px; }
  </style>
</head>
<body>
  <h2>💡 LED CONTROL ROOM</h2>
  
  <button class="btn on" onclick="send('on')">BẬT ĐÈN</button>
  <button class="btn off" onclick="send('off')">TẮT ĐÈN</button>

  <a href="/" class="home">⬅ Quay về trang chủ</a>

<script>
  var ws = new WebSocket('ws://' + location.host + '/ws');
  function send(cmd) { ws.send('led:' + cmd); } // Gửi lệnh kèm prefix "led:" để phân biệt
</script>
</body>
</html>
)rawliteral";