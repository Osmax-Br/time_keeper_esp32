const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
      .pause_button {
        padding: 10px 20px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #2f4468;
        border: none;
        border-radius: 5px;
        box-shadow: 0 6px #999;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .pause_button:hover {background-color: #1f2e45}
      .pause_button:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
  </style>
</head>
<body>
  <h1>Time Keeper</h1>
  <p>
  <h2>
    <span id="timer">%Timer%</span>
  </h2>  
  </p>
  <p><h3>chosen : <span id="chosen_server">%Chosen_server%</span></h3></p>
  <p><button id = "btn" class = "pause_button" onclick="toggleCheckbox()"><span id="btn">%Pause%</button></p>
  <form action="/choseee">
    choose activity : <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>

</body>
<script>

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("timer").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/timer", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("chosen_server").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/chosen_server", true);
  xhttp.send();
}, 1000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("btn").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/btn", true);
  xhttp.send();
}, 1000 ) ;
function toggleCheckbox() {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/PauseBtn" , true);
     xhr.send();
   }
</script>
</html>)rawliteral";
/*
 <form action = "javascript:handleIt()">
  <input type="text" name="input1">
  <input name="Submit"  type="submit" value="send"/>
    </form>
*/
