#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char PAGE_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Number Station OS</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: 'Courier New', Courier, monospace; background-color: #050505; color: #00ff00; padding: 20px; text-shadow: 0 0 5px #00ff00; }
        h1 { color: #00ff00; border-bottom: 2px solid #00ff00; padding-bottom: 10px; text-align: center; }
        input[type="text"], input[type="number"], select { background-color: #111; color: #00ff00; border: 1px solid #00ff00; padding: 10px; margin: 10px 0; width: 100%; box-sizing: border-box; font-family: monospace; font-size: 16px; }
        button { background-color: #00ff00; color: #000; border: none; padding: 15px 20px; cursor: pointer; margin-top: 15px; font-weight: bold; width: 100%; font-size: 18px; text-transform: uppercase; }
        button:hover { background-color: #00cc00; }
        .panel { border: 1px dashed #00ff00; padding: 20px; margin-bottom: 20px; background-color: rgba(0, 255, 0, 0.05); }
        label { font-weight: bold; }
        .crt::before { content: " "; display: block; position: absolute; top: 0; left: 0; bottom: 0; right: 0; background: linear-gradient(rgba(18, 16, 16, 0) 50%, rgba(0, 0, 0, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06)); z-index: 2; background-size: 100% 2px, 3px 100%; pointer-events: none; }
    </style>
</head>
<body class="crt">
    <h1>NUMBER STATION OS</h1>

    <div class="panel">
        <h2>TRANSMISSION PROTOCOL</h2>

        <label>Mode:</label>
        <select id="broadcast_mode" onchange="toggleMode()">
            <option value="0">Tonal Number Sequence</option>
            <option value="1">Morse Code Cipher</option>
        </select>

        <div id="mode0_div">
            <label>Number Sequence (0-9):</label>
            <input type="text" id="sequence" placeholder="e.g. 5 4 3 2 1">
        </div>

        <div id="mode1_div" style="display:none;">
            <label>Morse Message:</label>
            <input type="text" id="morse_message" placeholder="e.g. THE EAGLE HAS LANDED">
            <label>WPM (Words Per Minute):</label>
            <input type="number" id="wpm" value="20" min="5" max="60">
        </div>

        <label>Transmission Interval (mins):</label>
        <input type="number" id="interval" value="1" min="1">

        <button onclick="saveSettings()">Save Protocol</button>
    </div>

    <div class="panel">
        <h2>STATION CONTROL</h2>
        <button onclick="startBroadcast()" style="background-color: #f00; color: #fff;">ACTIVATE TRANSMISSION</button>
        <button onclick="stopBroadcast()" style="background-color: #555; color: #fff;">ABORT TRANSMISSION</button>
        <p id="status" style="text-align:center; font-size: 20px; font-weight:bold; margin-top:20px;">Status: STANDBY</p>
    </div>

    <script>
        function toggleMode() {
            var mode = document.getElementById('broadcast_mode').value;
            if (mode == "0") {
                document.getElementById('mode0_div').style.display = "block";
                document.getElementById('mode1_div').style.display = "none";
            } else {
                document.getElementById('mode0_div').style.display = "none";
                document.getElementById('mode1_div').style.display = "block";
            }
        }
        function saveSettings() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/save_config", true);
            xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
            var params = "sequence=" + encodeURIComponent(document.getElementById('sequence').value) +
                         "&morse_message=" + encodeURIComponent(document.getElementById('morse_message').value) +
                         "&interval=" + document.getElementById('interval').value +
                         "&broadcast_mode=" + document.getElementById('broadcast_mode').value +
                         "&wpm=" + document.getElementById('wpm').value;
            xhr.send(params);
            xhr.onload = function() { alert("PROTOCOL SAVED"); }
        }
        function startBroadcast() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/start", true);
            xhr.send();
            document.getElementById('status').innerText = "Status: TRANSMITTING";
            document.getElementById('status').style.color = "#f00";
        }
        function stopBroadcast() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/stop", true);
            xhr.send();
            document.getElementById('status').innerText = "Status: STANDBY";
            document.getElementById('status').style.color = "#0f0";
        }
    </script>
</body>
</html>
)=====";

#endif // WEB_PAGES_H
