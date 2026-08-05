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
        body { font-family: monospace; background-color: #111; color: #0f0; padding: 20px; }
        h1 { color: #0f0; }
        input[type="text"], input[type="number"] { background-color: #222; color: #0f0; border: 1px solid #0f0; padding: 5px; margin: 5px 0; width: 100%; box-sizing: border-box; }
        button { background-color: #0f0; color: #111; border: none; padding: 10px 20px; cursor: pointer; margin-top: 10px; font-weight: bold; }
        button:hover { background-color: #afa; }
        .panel { border: 1px solid #0f0; padding: 15px; margin-bottom: 20px; }
    </style>
</head>
<body>
    <h1>NUMBER STATION OS</h1>

    <div class="panel">
        <h2>Broadcast Sequence</h2>
        <input type="text" id="sequence" placeholder="e.g. 5 4 3 2 1">
        <br>
        <label>Interval (mins):</label>
        <input type="number" id="interval" value="1" min="1">
        <br>
        <label>Pitch (0-255):</label>
        <input type="number" id="pitch" value="64" min="0" max="255">
        <br>
        <label>Speed (0-255):</label>
        <input type="number" id="speed" value="72" min="0" max="255">
        <br>
        <button onclick="saveSettings()">Save Configuration</button>
    </div>

    <div class="panel">
        <h2>Control</h2>
        <button onclick="startBroadcast()">START BROADCAST</button>
        <button onclick="stopBroadcast()">STOP</button>
        <p id="status">Status: Idle</p>
    </div>

    <script>
        function saveSettings() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/save_config", true);
            xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
            var params = "sequence=" + encodeURIComponent(document.getElementById('sequence').value) +
                         "&interval=" + document.getElementById('interval').value +
                         "&pitch=" + document.getElementById('pitch').value +
                         "&speed=" + document.getElementById('speed').value;
            xhr.send(params);
            xhr.onload = function() { alert("Saved"); }
        }
        function startBroadcast() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/start", true);
            xhr.send();
            document.getElementById('status').innerText = "Status: Broadcasting";
        }
        function stopBroadcast() {
            var xhr = new XMLHttpRequest();
            xhr.open("POST", "/stop", true);
            xhr.send();
            document.getElementById('status').innerText = "Status: Idle";
        }
    </script>
</body>
</html>
)=====";

#endif // WEB_PAGES_H
