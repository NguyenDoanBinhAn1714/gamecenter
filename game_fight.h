const char game_fight_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Đại Chiến</title>
  <style>
    body { font-family: 'Segoe UI', sans-serif; background: #2c3e50; color: white; text-align: center; margin: 0; padding: 10px; user-select: none; }
    h2 { margin: 10px 0; color: #e74c3c; text-transform: uppercase; }

    /* Thanh máu */
    .hp-container { display: flex; justify-content: space-between; margin-bottom: 20px; padding: 0 10px; }
    .hp-box { width: 45%; }
    .hp-bar-bg { width: 100%; height: 20px; background: #555; border-radius: 10px; overflow: hidden; border: 2px solid #fff; }
    .hp-bar { height: 100%; width: 100%; transition: width 0.3s; }
    #hp1 { background: #e74c3c; } /* P1 màu đỏ */
    #hp2 { background: #3498db; } /* P2 màu xanh */

    /* Khu vực chiến đấu */
    .arena { height: 150px; display: flex; justify-content: center; align-items: center; gap: 20px; font-size: 60px; background: #34495e; border-radius: 15px; margin-bottom: 20px; }
    .vs-text { font-size: 30px; font-weight: bold; color: #f1c40f; }

    /* Nút bấm */
    .controls { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 10px; }
    .btn-move { 
      background: #ecf0f1; border: none; border-radius: 15px; padding: 20px 0; font-size: 40px; cursor: pointer; 
      box-shadow: 0 5px #bdc3c7; transition: transform 0.1s;
    }
    .btn-move:active { transform: translateY(4px); box-shadow: 0 2px #bdc3c7; }
    .btn-move.selected { background: #f1c40f; border: 4px solid #e67e22; }

    #status { font-size: 18px; margin-bottom: 10px; font-weight: bold; min-height: 25px; }
    .home { display: block; margin-top: 30px; color: #95a5a6; text-decoration: none; padding: 10px; border: 1px solid #95a5a6; border-radius: 5px; }

    /* Modal kết quả */
    #modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.9); flex-direction: column; justify-content: center; align-items: center; z-index: 99; }
    #win-msg { font-size: 40px; color: gold; font-weight: bold; margin-bottom: 20px; }
  </style>
</head>
<body>

  <h2>⚔️ ĐỐI KHÁNG ⚔️</h2>
  <div id="status">Đang kết nối...</div>

  <div class="hp-container">
    <div class="hp-box">
      <div>P1 (Red)</div>
      <div class="hp-bar-bg"><div id="hp1" class="hp-bar"></div></div>
    </div>
    <div class="hp-box">
      <div>P2 (Blue)</div>
      <div class="hp-bar-bg"><div id="hp2" class="hp-bar"></div></div>
    </div>
  </div>

  <div class="arena">
    <div id="move1">❔</div>
    <div class="vs-text">VS</div>
    <div id="move2">❔</div>
  </div>

  <div class="controls" id="ctrl-panel">
    <button class="btn-move" onclick="pick('rock')">🪨</button>
    <button class="btn-move" onclick="pick('paper')">📄</button>
    <button class="btn-move" onclick="pick('scissors')">✂️</button>
  </div>

  <a href="/" class="home">Thoát Game</a>

  <div id="modal">
    <div id="win-msg">WINNER!</div>
    <button class="btn-move" style="font-size: 20px; padding: 10px 30px;" onclick="resetGame()">CHƠI LẠI</button>
  </div>

<script>
  let ws = new WebSocket('ws://' + location.host + '/ws');
  let mySlot = -1;
  const moves = { 'rock': '🪨', 'paper': '📄', 'scissors': '✂️', 'none': '❔' };

  ws.onopen = () => { document.getElementById('status').innerText = "Chờ đối thủ..."; };

  ws.onmessage = (evt) => {
    let msg = evt.data;
    
    // Logic riêng cho game fight (Bắt đầu bằng fight:)
    if(msg.startsWith("fight:")) {
      let parts = msg.split(':');
      let cmd = parts[1];
      
      if(cmd === 'welcome') {
        mySlot = parseInt(parts[2]);
        document.getElementById('status').innerText = "Bạn là Player " + (mySlot + 1);
        document.getElementById('status').style.color = mySlot == 0 ? '#e74c3c' : '#3498db';
      }
      else if(cmd === 'update') {
        // fight:update:hp1:hp2:move1:move2:statusText
        let data = parts[2].split(',');
        // Cập nhật HP
        document.getElementById('hp1').style.width = data[0] + '%';
        document.getElementById('hp2').style.width = data[1] + '%';
        
        // Cập nhật Move (nếu server gửi về)
        document.getElementById('move1').innerText = moves[data[2]];
        document.getElementById('move2').innerText = moves[data[3]];
        
        // Thông báo
        if(data[4] !== 'null') document.getElementById('status').innerText = data[4];

        // Reset nút bấm
        let btns = document.getElementsByClassName('btn-move');
        for(let b of btns) b.classList.remove('selected');
      }
      else if(cmd === 'end') {
        document.getElementById('win-msg').innerText = "PLAYER " + (parseInt(parts[2]) + 1) + " THẮNG!";
        document.getElementById('modal').style.display = 'flex';
      }
    }
  };

  function pick(m) {
    if(mySlot !== -1) {
      ws.send('fight:' + m);
      // Hiệu ứng chọn nút
      let btns = document.getElementsByClassName('btn-move');
      for(let b of btns) b.classList.remove('selected');
      event.target.classList.add('selected');
      document.getElementById('status').innerText = "Đã chọn! Chờ đối thủ...";
    }
  }

  function resetGame() { 
    ws.send('fight:reset'); 
    document.getElementById('modal').style.display = 'none';
  }
</script>
</body>
</html>
)rawliteral";