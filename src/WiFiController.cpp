#include "WiFiController.h"
#include "ChatManager.h"
#include "Config.h"
#include "LED.h"
#include "WebPages.h"

// External OLED/display handle reference so we can adjust contrast live!
// We'll declare it as an extern pointer or function to be defined in main.cpp.
extern void updateOLEDContrast(uint8_t contrast);
extern LED systemLed;

void WiFiController::begin() {
    // Set WiFi to dual mode (both STA and AP)
    WiFi.mode(WIFI_AP_STA);

    // 1. Start Access Point
    startAP();

    // 2. Start Captive Portal DNS Server
    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

    // 3. Setup Web Server routes
    setupWebServer();

    // 4. If we have saved WiFi SSID, try to connect asynchronously
    SystemSettings& settings = Config::getInstance().getSettings();
    if (strlen(settings.wifiSSID) > 0) {
        connectToSTA(settings.wifiSSID, settings.wifiPass);
    }
}

void WiFiController::startAP() {
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    // Start AP with open network
    WiFi.softAP("Local-Chat-OS");
}

void WiFiController::connectToSTA(const char* ssid, const char* pass) {
    WiFi.begin(ssid, pass);
    _staConnecting = true;
    _staConnectStartTime = millis();
}

void WiFiController::disconnectSTA() {
    WiFi.disconnect();
    _staConnecting = false;
}

void WiFiController::startScan() {
    if (_isScanning) return;
    _isScanning = true;
    _lastScanTime = millis();
    WiFi.scanNetworks(true); // Async scan
}

void WiFiController::update() {
    _dnsServer.processNextRequest();
    _webServer.handleClient();

    // Handle asynchronous WiFi scanning
    if (_isScanning) {
        int16_t scanResult = WiFi.scanComplete();
        if (scanResult >= 0) {
            _isScanning = false;
            _scannedNetworks.clear();

            // Limit to max 12 networks to protect memory
            int limit = (scanResult > 12) ? 12 : scanResult;
            for (int i = 0; i < limit; i++) {
                ScannedNetwork net;
                strncpy(net.ssid, WiFi.SSID(i).c_str(), sizeof(net.ssid) - 1);
                net.ssid[sizeof(net.ssid) - 1] = '\0';
                net.rssi = WiFi.RSSI(i);
                net.encryptionType = WiFi.encryptionType(i);
                _scannedNetworks.push_back(net);
            }
            WiFi.scanDelete();
        } else if (scanResult == WIFI_SCAN_FAILED || (millis() - _lastScanTime > 15000)) {
            _isScanning = false;
            WiFi.scanDelete();
        }
    }

    // Monitor STA connection progress
    _staStatus = WiFi.status();
    if (_staConnecting) {
        if (_staStatus == WL_CONNECTED) {
            _staConnecting = false;
        } else if (_staStatus == WL_CONNECT_FAILED || _staStatus == WL_NO_SSID_AVAIL || (millis() - _staConnectStartTime > 20000)) {
            _staConnecting = false;
        }
    }
}

uint8_t WiFiController::getAPClientCount() const {
    return WiFi.softAPgetStationNum();
}

// Check if a request is from a captive portal device
bool WiFiController::isCaptivePortalRequest() {
    if (!_webServer.hasHeader("Host")) return false;
    String host = _webServer.header("Host");
    if (host != "192.168.4.1" && host != "localhost") {
        return true;
    }
    return false;
}

void WiFiController::setupWebServer() {
    // Collect Host and User-Agent headers
    _webServer.collectHeaders("Host", "User-Agent");

    _webServer.on("/", HTTP_GET, std::bind(&WiFiController::handleRoot, this));
    _webServer.on("/messages", HTTP_GET, std::bind(&WiFiController::handleGetMessages, this));
    _webServer.on("/send", HTTP_GET, std::bind(&WiFiController::handleSendMessage, this));
    _webServer.on("/scan", HTTP_GET, std::bind(&WiFiController::handleScanResults, this));
    _webServer.on("/connect", HTTP_GET, std::bind(&WiFiController::handleConnectSTA, this));
    _webServer.on("/status", HTTP_GET, std::bind(&WiFiController::handleGetStatus, this));
    _webServer.on("/save-config", HTTP_GET, std::bind(&WiFiController::handleSaveConfig, this));

    // Redirect portal capture requests to root
    _webServer.on("/generate_204", std::bind(&WiFiController::handleRoot, this));
    _webServer.on("/fwlink", std::bind(&WiFiController::handleRoot, this));
    _webServer.on("/hotspot-detect.html", std::bind(&WiFiController::handleRoot, this));
    _webServer.on("/success.txt", std::bind(&WiFiController::handleRoot, this));

    _webServer.onNotFound(std::bind(&WiFiController::handleNotFound, this));

    _webServer.begin();
}

void WiFiController::handleRoot() {
    if (isCaptivePortalRequest()) {
        _webServer.sendHeader("Location", "http://192.168.4.1/", true);
        _webServer.send(302, "text/plain", "Redirecting to Captive Portal");
        return;
    }

    _webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    _webServer.sendHeader("Pragma", "no-cache");
    _webServer.sendHeader("Expires", "-1");
    _webServer.send_P(200, "text/html", INDEX_HTML);
}

void WiFiController::handleGetMessages() {
    // Construct lightweight JSON of messages
    const auto& msgs = ChatManager::getInstance().getMessages();
    String json = "[";
    uint32_t nowSec = millis() / 1000;

    for (size_t i = 0; i < msgs.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"id\":" + String(msgs[i].id) + ",";
        json += "\"nickname\":\"" + String(msgs[i].nickname) + "\",";

        // Escape quotes and backslashes in JSON string
        String textEscaped = "";
        for (size_t j = 0; msgs[i].text[j] != '\0'; j++) {
            char c = msgs[i].text[j];
            if (c == '"') textEscaped += "\\\"";
            else if (c == '\\') textEscaped += "\\\\";
            else textEscaped += c;
        }
        json += "\"text\":\"" + textEscaped + "\",";

        uint32_t elapsed = (nowSec >= msgs[i].timestampSec) ? (nowSec - msgs[i].timestampSec) : 0;
        json += "\"time\":" + String(elapsed);
        json += "}";
    }
    json += "]";

    _webServer.sendHeader("Cache-Control", "no-cache");
    _webServer.send(200, "application/json", json);
}

void WiFiController::handleSendMessage() {
    if (!_webServer.hasArg("nickname") || !_webServer.hasArg("text")) {
        _webServer.send(400, "text/plain", "Missing arguments");
        return;
    }

    String nick = _webServer.arg("nickname");
    String text = _webServer.arg("text");

    if (nick.length() > 16) nick = nick.substring(0, 16);
    if (text.length() > 64) text = text.substring(0, 64);

    ChatManager::getInstance().addMessage(nick.c_str(), text.c_str());

    // Trigger double quick flash on message receive as positive feedback!
    if (systemLed.getMode() == LED_BREATHE) {
        systemLed.setMode(LED_FAST_BLINK);
    }

    _webServer.send(200, "application/json", "{\"status\":\"ok\"}");
}

void WiFiController::handleScanResults() {
    // If not scanning and scanned list is empty, start scan
    if (!_isScanning && _scannedNetworks.empty()) {
        startScan();
        _webServer.send(200, "application/json", "[]");
        return;
    }

    if (_isScanning) {
        _webServer.send(200, "application/json", "[]");
        return;
    }

    // Output JSON array of scanned networks
    String json = "[";
    for (size_t i = 0; i < _scannedNetworks.size(); i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + String(_scannedNetworks[i].ssid) + "\",";
        json += "\"rssi\":" + String(_scannedNetworks[i].rssi) + ",";
        json += "\"enc\":" + String(_scannedNetworks[i].encryptionType);
        json += "}";
    }
    json += "]";

    _webServer.send(200, "application/json", json);
}

void WiFiController::handleConnectSTA() {
    if (!_webServer.hasArg("ssid")) {
        _webServer.send(400, "text/plain", "Missing SSID");
        return;
    }

    String ssid = _webServer.arg("ssid");
    String pass = _webServer.hasArg("pass") ? _webServer.arg("pass") : "";

    SystemSettings& settings = Config::getInstance().getSettings();
    strncpy(settings.wifiSSID, ssid.c_str(), sizeof(settings.wifiSSID) - 1);
    settings.wifiSSID[sizeof(settings.wifiSSID) - 1] = '\0';

    strncpy(settings.wifiPass, pass.c_str(), sizeof(settings.wifiPass) - 1);
    settings.wifiPass[sizeof(settings.wifiPass) - 1] = '\0';

    Config::getInstance().save();

    connectToSTA(settings.wifiSSID, settings.wifiPass);

    _webServer.send(200, "text/plain", "Connection started...");
}

void WiFiController::handleGetStatus() {
    uint32_t uptimeSec = millis() / 1000;
    SystemSettings& settings = Config::getInstance().getSettings();

    String staState = "Disconnected";
    if (_staConnecting) {
        staState = "Connecting...";
    } else if (_staStatus == WL_CONNECTED) {
        staState = "Connected (" + WiFi.localIP().toString() + ")";
    } else if (_staStatus == WL_CONNECT_FAILED) {
        staState = "Connection Failed";
    }

    String json = "{";
    json += "\"clients\":" + String(getAPClientCount()) + ",";
    json += "\"uptime\":" + String(uptimeSec) + ",";
    json += "\"sta\":\"" + staState + "\",";
    json += "\"contrast\":" + String(settings.contrast) + ",";
    json += "\"ledMode\":" + String(settings.ledSetting) + ",";
    json += "\"screensaver\":" + String(settings.screenSaverMins);
    json += "}";

    _webServer.send(200, "application/json", json);
}

void WiFiController::handleSaveConfig() {
    SystemSettings& settings = Config::getInstance().getSettings();
    bool changed = false;

    if (_webServer.hasArg("reset")) {
        Config::getInstance().resetToDefaults();
        // Update OLED Contrast live
        updateOLEDContrast(settings.contrast);
        // Update LED setting live
        systemLed.setMode((LEDMode)settings.ledSetting);
        _webServer.send(200, "text/plain", "Reset successfully");
        return;
    }

    if (_webServer.hasArg("clearchat")) {
        ChatManager::getInstance().clear();
        _webServer.send(200, "text/plain", "Chat cleared");
        return;
    }

    if (_webServer.hasArg("ledMode")) {
        uint8_t mode = _webServer.arg("ledMode").toInt();
        settings.ledSetting = mode;
        systemLed.setMode((LEDMode)mode);
        changed = true;
    }

    if (_webServer.hasArg("contrast")) {
        uint8_t contrast = _webServer.arg("contrast").toInt();
        settings.contrast = contrast;
        updateOLEDContrast(contrast);
        changed = true;
    }

    if (_webServer.hasArg("screensaver")) {
        uint8_t ss = _webServer.arg("screensaver").toInt();
        settings.screenSaverMins = ss;
        changed = true;
    }

    if (changed) {
        Config::getInstance().save();
    }

    _webServer.send(200, "text/plain", "Saved");
}

void WiFiController::handleNotFound() {
    if (isCaptivePortalRequest()) {
        _webServer.sendHeader("Location", "http://192.168.4.1/", true);
        _webServer.send(302, "text/plain", "Redirecting to Captive Portal");
    } else {
        _webServer.send(404, "text/plain", "File Not Found");
    }
}
