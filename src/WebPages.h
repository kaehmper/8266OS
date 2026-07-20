#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP8266 Local OS Chat</title>
    <style>
        :root {
            --bg-color: #0d1117;
            --container-bg: #161b22;
            --border-color: #30363d;
            --accent-color: #58a6ff;
            --text-color: #c9d1d9;
            --text-muted: #8b949e;
            --btn-bg: #21262d;
            --btn-hover: #30363d;
            --success-color: #2ea44f;
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
        }
        body {
            background-color: var(--bg-color);
            color: var(--text-color);
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
            padding: 15px;
        }
        .container {
            width: 100%;
            max-width: 500px;
            background-color: var(--container-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            overflow: hidden;
            box-shadow: 0 8px 24px rgba(0,0,0,0.5);
            display: flex;
            flex-direction: column;
        }
        header {
            padding: 15px;
            background-color: #1f242c;
            border-bottom: 1px solid var(--border-color);
            text-align: center;
        }
        header h1 {
            font-size: 1.3rem;
            color: var(--accent-color);
            margin-bottom: 5px;
        }
        .stats {
            font-size: 0.8rem;
            color: var(--text-muted);
            display: flex;
            justify-content: space-around;
            padding: 5px 0;
            border-top: 1px solid rgba(255,255,255,0.05);
            margin-top: 5px;
        }
        .nav-tabs {
            display: flex;
            background-color: #12161c;
            border-bottom: 1px solid var(--border-color);
        }
        .tab-btn {
            flex: 1;
            padding: 12px;
            background: none;
            border: none;
            color: var(--text-muted);
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
            text-align: center;
            font-size: 0.9rem;
        }
        .tab-btn:hover {
            color: var(--text-color);
            background-color: rgba(255,255,255,0.02);
        }
        .tab-btn.active {
            color: var(--accent-color);
            border-bottom: 2px solid var(--accent-color);
            background-color: rgba(88,166,255,0.05);
        }
        .tab-content {
            padding: 15px;
            display: none;
            height: 380px;
            overflow-y: auto;
        }
        .tab-content.active {
            display: flex;
            flex-direction: column;
        }
        /* Chat View */
        #chat-view {
            display: none; /* overrides basic tab behavior to allow separate flex layout */
        }
        #chat-view.active {
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }
        .chat-messages {
            flex-grow: 1;
            overflow-y: auto;
            border: 1px solid var(--border-color);
            border-radius: 8px;
            padding: 10px;
            background-color: #0d1117;
            margin-bottom: 10px;
            display: flex;
            flex-direction: column;
            gap: 8px;
            max-height: 280px;
        }
        .message-bubble {
            padding: 6px 10px;
            border-radius: 8px;
            max-width: 85%;
            word-break: break-word;
            background-color: #21262d;
            align-self: flex-start;
        }
        .message-bubble.me {
            background-color: #1f6feb;
            color: white;
            align-self: flex-end;
        }
        .msg-header {
            font-size: 0.75rem;
            color: var(--accent-color);
            font-weight: bold;
            margin-bottom: 2px;
            display: flex;
            justify-content: space-between;
        }
        .message-bubble.me .msg-header {
            color: #c9d1d9;
        }
        .msg-text {
            font-size: 0.9rem;
        }
        .chat-input-area {
            display: flex;
            gap: 8px;
        }
        .chat-input-area input {
            background-color: #0d1117;
            border: 1px solid var(--border-color);
            border-radius: 6px;
            color: var(--text-color);
            padding: 8px 12px;
            outline: none;
            font-size: 0.9rem;
        }
        #nickname {
            width: 30%;
        }
        #message {
            flex-grow: 1;
        }
        .btn {
            background-color: var(--btn-bg);
            border: 1px solid var(--border-color);
            color: var(--text-color);
            border-radius: 6px;
            padding: 8px 14px;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.2s;
            font-size: 0.9rem;
        }
        .btn:hover {
            background-color: var(--btn-hover);
        }
        .btn-primary {
            background-color: var(--success-color);
            border-color: #2c974b;
            color: white;
        }
        .btn-primary:hover {
            background-color: #2c974b;
        }
        /* WiFi Screen */
        .wifi-list {
            display: flex;
            flex-direction: column;
            gap: 8px;
            margin-bottom: 15px;
            max-height: 180px;
            overflow-y: auto;
            border: 1px solid var(--border-color);
            border-radius: 8px;
            padding: 8px;
            background-color: #0d1117;
        }
        .wifi-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 8px 12px;
            border-radius: 6px;
            background-color: #161b22;
            border: 1px solid var(--border-color);
            cursor: pointer;
            transition: background-color 0.2s;
        }
        .wifi-item:hover {
            background-color: #21262d;
        }
        .wifi-item.selected {
            border-color: var(--accent-color);
            background-color: rgba(88,166,255,0.05);
        }
        .wifi-info {
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .wifi-ssid {
            font-weight: 600;
            font-size: 0.9rem;
        }
        .wifi-rssi {
            font-size: 0.8rem;
            color: var(--text-muted);
        }
        .form-group {
            display: flex;
            flex-direction: column;
            gap: 6px;
            margin-bottom: 12px;
        }
        .form-group label {
            font-size: 0.85rem;
            color: var(--text-muted);
            font-weight: 600;
        }
        .form-group input, .form-group select {
            background-color: #0d1117;
            border: 1px solid var(--border-color);
            color: var(--text-color);
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 0.9rem;
            outline: none;
        }
        /* Settings Tab */
        .range-value {
            display: flex;
            justify-content: space-between;
            font-size: 0.85rem;
            color: var(--text-muted);
        }
        .no-networks {
            text-align: center;
            padding: 20px;
            color: var(--text-muted);
            font-size: 0.9rem;
        }
    </style>
</head>
<body>

<div class="container">
    <header>
        <h1>Local Chat OS v1.0</h1>
        <div class="stats">
            <span>Clients: <strong id="stat-clients">0</strong></span>
            <span>Uptime: <strong id="stat-uptime">0s</strong></span>
            <span>WiFi STA: <strong id="stat-sta">Disconnect</strong></span>
        </div>
    </header>

    <div class="nav-tabs">
        <button class="tab-btn active" onclick="switchTab('chat')">💬 Chat</button>
        <button class="tab-btn" onclick="switchTab('wifi'); refreshWiFi();">📶 WiFi Setup</button>
        <button class="tab-btn" onclick="switchTab('settings')">⚙️ Options</button>
    </div>

    <!-- Chat Room Tab -->
    <div id="chat-view" class="tab-content active">
        <div class="chat-messages" id="chat-box">
            <div class="message-bubble">
                <div class="msg-header">System</div>
                <div class="msg-text">Welcome to the ESP8266 Local Chat Room!</div>
            </div>
        </div>
        <div class="chat-input-area">
            <input type="text" id="nickname" placeholder="Nick" maxlength="15" value="Guest">
            <input type="text" id="message" placeholder="Type a message..." maxlength="60" onkeydown="if(event.key==='Enter') sendMessage()">
            <button class="btn btn-primary" onclick="sendMessage()">Send</button>
        </div>
    </div>

    <!-- WiFi Setup Tab -->
    <div id="wifi-view" class="tab-content">
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:10px;">
            <h3 style="font-size:1rem;">Networks in Range</h3>
            <button class="btn" style="padding:4px 8px; font-size:0.8rem;" onclick="refreshWiFi()">🔄 Scan</button>
        </div>
        <div class="wifi-list" id="ap-list">
            <div class="no-networks">Scanning networks...</div>
        </div>
        <div class="form-group">
            <label for="wifi-ssid">Selected SSID</label>
            <input type="text" id="wifi-ssid" placeholder="Choose a network or type SSID">
        </div>
        <div class="form-group">
            <label for="wifi-pass">WiFi Password</label>
            <input type="password" id="wifi-pass" placeholder="Network password">
        </div>
        <button class="btn btn-primary" style="margin-top:5px;" onclick="connectWiFi()">Connect & Save</button>
    </div>

    <!-- Settings Tab -->
    <div id="settings-view" class="tab-content">
        <h3 style="font-size:1rem; margin-bottom:12px; border-bottom:1px solid var(--border-color); padding-bottom:5px;">Hardware Controls</h3>
        <div class="form-group">
            <label for="led-mode">RED LED Mode</label>
            <select id="led-mode" onchange="saveOptions()">
                <option value="0">Always Off</option>
                <option value="1">Always On</option>
                <option value="2">Blink Alert</option>
                <option value="3">Pulse Breathe</option>
            </select>
        </div>
        <div class="form-group">
            <div class="range-value">
                <label for="contrast">OLED Screen Contrast</label>
                <span id="contrast-val">127</span>
            </div>
            <input type="range" id="contrast" min="0" max="255" value="127" oninput="updateContrastVal(this.value)" onchange="saveOptions()">
        </div>
        <div class="form-group">
            <label for="screensaver">OLED Screen Saver</label>
            <select id="screensaver" onchange="saveOptions()">
                <option value="0">Never sleep</option>
                <option value="1">1 Minute</option>
                <option value="2">2 Minutes</option>
                <option value="5">5 Minutes</option>
            </select>
        </div>
        <div style="margin-top:15px; border-top:1px solid var(--border-color); padding-top:15px; display:flex; gap:10px;">
            <button class="btn" style="flex:1; border-color:#da3637; color:#da3637;" onclick="resetConfig()">Reset Config</button>
            <button class="btn" style="flex:1;" onclick="clearChat()">Clear Chat</button>
        </div>
    </div>
</div>

<script>
    let lastMsgId = 0;
    let selectedSSID = "";

    function switchTab(tab) {
        document.querySelectorAll('.tab-btn').forEach(btn => btn.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));

        event.target.classList.add('active');
        document.getElementById(tab + '-view').classList.add('active');
    }

    function updateContrastVal(val) {
        document.getElementById('contrast-val').innerText = val;
    }

    // Load dynamic parameters
    function fetchStatus() {
        fetch('/status')
            .then(res => res.json())
            .then(data => {
                document.getElementById('stat-clients').innerText = data.clients;
                document.getElementById('stat-uptime').innerText = data.uptime + "s";
                document.getElementById('stat-sta').innerText = data.sta;

                // Update settings values if they haven't been edited
                if(!document.getElementById('contrast').dataset.edited) {
                    document.getElementById('contrast').value = data.contrast;
                    document.getElementById('contrast-val').innerText = data.contrast;
                }
                if(!document.getElementById('led-mode').dataset.edited) {
                    document.getElementById('led-mode').value = data.ledMode;
                }
                if(!document.getElementById('screensaver').dataset.edited) {
                    document.getElementById('screensaver').value = data.screensaver;
                }
            });
    }

    // Refresh chat messages
    function fetchMessages() {
        fetch('/messages')
            .then(res => res.json())
            .then(messages => {
                const chatBox = document.getElementById('chat-box');
                let newMessages = false;

                messages.forEach(msg => {
                    if (msg.id > lastMsgId) {
                        const isMe = (msg.nickname === document.getElementById('nickname').value && msg.id > 1);
                        const bubble = document.createElement('div');
                        bubble.className = 'message-bubble' + (isMe ? ' me' : '');

                        bubble.innerHTML = `
                            <div class="msg-header">
                                <span>${escapeHtml(msg.nickname)}</span>
                                <span style="font-size:0.65rem; color:var(--text-muted); margin-left:10px;">${msg.time}s ago</span>
                            </div>
                            <div class="msg-text">${escapeHtml(msg.text)}</div>
                        `;
                        chatBox.appendChild(bubble);
                        lastMsgId = msg.id;
                        newMessages = true;
                    }
                });

                if (newMessages) {
                    chatBox.scrollTop = chatBox.scrollHeight;
                }
            });
    }

    function sendMessage() {
        const nickInput = document.getElementById('nickname');
        const msgInput = document.getElementById('message');
        const nick = nickInput.value.trim() || "Guest";
        const text = msgInput.value.trim();

        if (!text) return;

        msgInput.value = "";

        fetch(`/send?nickname=${encodeURIComponent(nick)}&text=${encodeURIComponent(text)}`)
            .then(() => {
                fetchMessages();
            });
    }

    function refreshWiFi() {
        const apList = document.getElementById('ap-list');
        apList.innerHTML = '<div class="no-networks">Scanning nearby networks...</div>';

        fetch('/scan')
            .then(res => res.json())
            .then(networks => {
                apList.innerHTML = '';
                if (networks.length === 0) {
                    apList.innerHTML = '<div class="no-networks">No networks found or scanning. Try again.</div>';
                    return;
                }

                networks.forEach(net => {
                    const item = document.createElement('div');
                    item.className = 'wifi-item' + (selectedSSID === net.ssid ? ' selected' : '');
                    item.onclick = () => selectNetwork(net.ssid);

                    item.innerHTML = `
                        <div class="wifi-info">
                            <span class="wifi-ssid">${escapeHtml(net.ssid)}</span>
                        </div>
                        <span class="wifi-rssi">${net.rssi} dBm (${getSignalPercent(net.rssi)}%)</span>
                    `;
                    apList.appendChild(item);
                });
            });
    }

    function selectNetwork(ssid) {
        selectedSSID = ssid;
        document.getElementById('wifi-ssid').value = ssid;
        document.querySelectorAll('.wifi-item').forEach(item => {
            if (item.querySelector('.wifi-ssid').innerText === ssid) {
                item.classList.add('selected');
            } else {
                item.classList.remove('selected');
            }
        });
    }

    function connectWiFi() {
        const ssid = document.getElementById('wifi-ssid').value;
        const pass = document.getElementById('wifi-pass').value;

        if (!ssid) {
            alert("Please select or enter an SSID.");
            return;
        }

        fetch(`/connect?ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(pass)}`)
            .then(res => res.text())
            .then(msg => {
                alert(msg);
            });
    }

    function saveOptions() {
        const ledMode = document.getElementById('led-mode').value;
        const contrast = document.getElementById('contrast').value;
        const screensaver = document.getElementById('screensaver').value;

        // Mark as edited to prevent immediate status fetch overwrite
        document.getElementById('led-mode').dataset.edited = true;
        document.getElementById('contrast').dataset.edited = true;
        document.getElementById('screensaver').dataset.edited = true;

        fetch(`/save-config?ledMode=${ledMode}&contrast=${contrast}&screensaver=${screensaver}`)
            .then(() => {
                // Remove edited tags after short delay
                setTimeout(() => {
                    delete document.getElementById('led-mode').dataset.edited;
                    delete document.getElementById('contrast').dataset.edited;
                    delete document.getElementById('screensaver').dataset.edited;
                }, 1500);
            });
    }

    function resetConfig() {
        if (confirm("Are you sure you want to restore default OS settings?")) {
            fetch('/save-config?reset=1')
                .then(() => {
                    alert("Settings restored to defaults!");
                    location.reload();
                });
        }
    }

    function clearChat() {
        if (confirm("Are you sure you want to clear local chat history?")) {
            fetch('/save-config?clearchat=1')
                .then(() => {
                    document.getElementById('chat-box').innerHTML = '';
                    lastMsgId = 0;
                    fetchMessages();
                });
        }
    }

    function getSignalPercent(rssi) {
        if (rssi >= -50) return 100;
        if (rssi <= -100) return 0;
        return 2 * (rssi + 100);
    }

    function escapeHtml(text) {
        return text
            .replace(/&/g, "&amp;")
            .replace(/</g, "&lt;")
            .replace(/>/g, "&gt;")
            .replace(/"/g, "&quot;")
            .replace(/'/g, "&#039;");
    }

    // Event loops
    fetchStatus();
    fetchMessages();
    setInterval(fetchStatus, 3000);
    setInterval(fetchMessages, 2500);
</script>
</body>
</html>
)rawhtml";

#endif // WEB_PAGES_H
