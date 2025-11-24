const char game_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Đua Tốc Độ</title>
  <style>
    body { font-family: sans-serif; background: #2d3436; color: white; text-align: center; margin: 0; padding: 10px; user-select: none; }
    
    /* Thanh Header có nút Quay lại */
    .header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 20px; }
    .btn-home { background: #555; color: white; text-decoration: none; padding: 5px 15px; border-radius: 5px; font-size: 14px; }
    
    h2 { color: #ffeaa7; margin: 0; text-transform: uppercase; }
    
    .track { background: #636e72; border-radius: 10px; margin: 10px 0; height: 35px; border: 2px solid #b2bec3; position: relative; overflow: hidden; }
    .bar { height: 100%; width: 0%; line-height: 35px; text-align: right; padding-right: 10px; font-weight: bold; transition: width 0.1s linear; color: #2d3436; }
    
    .p0 { background: #ff7675; } 
    .p1 { background: #55efc4; } 
    .p2 { background: #74b9ff; } 
    .p3 { background: #fdcb6e; }

    #info { font-size: 18px; font-weight: bold; padding: 10px; background: #333; border-radius: 8px; margin-bottom: 10px; }
    
    #btn-tap { 
      width: 100%; max-width: 400px; height: 150px; border: none; border-radius: 20px; 
      background: radial-gradient(circle, #ff7675 0%, #d63031 100%);
      color: white; font-size: 40px; font-weight: 900; 
      box-shadow: 0 10px #b2bec3; cursor: pointer;
      -webkit-tap-highlight-color: transparent;
    }
    #btn-tap:active { transform: translateY(5px); box-shadow: 0 5px #b2bec3; }
    #btn-tap:disabled { background: #636e72; color: #b2bec3; box-shadow: none; cursor: not-allowed; }

    #modal { display: none; position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.9); flex-direction: column; justify-content: center; align-items: center; z-index: 99; }
    #win-txt { font-size: 50px; color: gold; font-weight: bold; margin-bottom: 30px; }
    button.small { padding: 10px 20px; font-size: 16px; border-radius: 5px; border: none; cursor: pointer; }
  </style>
</head>
<body>

  <div class="header">
    <a href="/" class="btn-home">⬅ Trang chủ</a>
    <h2>ĐUA XE</h2>
    <div style="width: 50px;"></div> </div>

  <div id="info">Đang kết nối...</div>

  <div id="bars">
    <div class="track"><div id="b0" class="bar p0">P1</div></div>
    <div class="track"><div id="b1" class="bar p1">P2</div></div>
    <div class="track"><div id="b2" class="bar p2">P3</div></div>
    <div class="track"><div id="b3" class="bar p3">P4</div></div>
  </div>

  <button id="btn-tap" onclick="tap()" ontouchstart="tap(event)">BẤM NHANH!</button>

  <div id="modal">
    <div id="win-txt">WINNER!</div>
    <button class="small" onclick="resetGame()" style="background: #00b894; color: white;">CHƠI LẠI</button>
    <br><br>
    <a href="/" class="small" style="background: #555; color: white; text-decoration: none; padding: 10px 20px; border-radius: 5px;">THOÁT RA</a>
  </div>

<script>
  let ws, mySlot = -1;
  function init() {
    ws = new WebSocket('ws://' + location.host + '/ws');
    ws.onopen = () => { document.getElementById('info').innerText = "Đã vào phòng!"; };
    
    ws.onmessage = (evt) => {
      let d = evt.data.split(',');
      if(d[0] === 'welcome') {
        mySlot = parseInt(d[1]);
        document.getElementById('info').innerText = "BẠN LÀ PLAYER " + (mySlot + 1);
        document.getElementById('info').style.color = getColor(mySlot);
      } 
      else if(d[0] === 'update') {
        for(let i=0; i<4; i++) {
          let s = parseInt(d[i+1]);
          let w = s > 100 ? 100 : s;
          document.getElementById('b'+i).style.width = w + '%';
          document.getElementById('b'+i).innerText = s;
        }
        document.getElementById('modal').style.display = 'none';
        document.getElementById('btn-tap').disabled = false;
      }
      else if(d[0] === 'win') {
        document.getElementById('win-txt').innerText = "PLAYER " + (parseInt(d[1])+1) + " THẮNG!";
        document.getElementById('modal').style.display = 'flex';
        if(navigator.vibrate) navigator.vibrate([200,100,200]);
      }
      else if(d[0] === 'full') {
        document.getElementById('info').innerText = "Phòng đầy! Chỉ xem.";
        document.getElementById('btn-tap').disabled = true;
      }
    };
    ws.onclose = () => setTimeout(init, 1000);
  }

  function tap(e) {
    if(e) e.preventDefault();
    if(mySlot !== -1 && ws.readyState === 1) ws.send('tap');
  }
  function resetGame() { ws.send('reset'); }
  function getColor(i) { return ['#ff7675','#55efc4','#74b9ff','#fdcb6e'][i]; }
  
  init();
</script>
</body>
</html>
)rawliteral";