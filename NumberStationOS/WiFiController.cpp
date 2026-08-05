#include "WiFiController.h"
#include "Config.h"
#include "WebPages.h"

void WiFiController::begin() {
    SystemSettings& settings = Config::getInstance().getSettings();
    startAP();

    if (strlen(settings.staSSID) > 0) {
        connectToSTA(settings.staSSID, settings.staPass);
    }

    setupWebServer();
}

void WiFiController::update() {
    _dnsServer.processNextRequest();
    _webServer.handleClient();

    if (_staConnecting) {
        _staStatus = WiFi.status();
        if (_staStatus == WL_CONNECTED || (millis() - _staConnectStartTime > 15000)) {
            _staConnecting = false;
        }
    }
}

void WiFiController::startAP() {
    SystemSettings& settings = Config::getInstance().getSettings();
    WiFi.mode(WIFI_AP_STA);

    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String defaultSSID = "OS_" + mac.substring(6);

    const char* ssidToUse = (strlen(settings.apSSID) > 0) ? settings.apSSID : defaultSSID.c_str();
    const char* passToUse = (strlen(settings.apPass) > 0) ? settings.apPass : NULL;

    WiFi.softAP(ssidToUse, passToUse);
    _dnsServer.start(53, "*", WiFi.softAPIP());
}

void WiFiController::connectToSTA(const char* ssid, const char* pass) {
    WiFi.begin(ssid, pass);
    _staConnecting = true;
    _staConnectStartTime = millis();
}

void WiFiController::disconnectSTA() {
    WiFi.disconnect();
    _staStatus = WL_DISCONNECTED;
    _staConnecting = false;
}

void WiFiController::startScan() {
    if (_isScanning) return;
    _isScanning = true;
    WiFi.scanNetworks(true); // Async scan
}

uint8_t WiFiController::getAPClientCount() const {
    return WiFi.softAPgetStationNum();
}

void WiFiController::setupWebServer() {
    _webServer.on("/", HTTP_GET, std::bind(&WiFiController::handleRoot, this));
    _webServer.on("/scan_results", HTTP_GET, std::bind(&WiFiController::handleScanResults, this));
    _webServer.on("/connect", HTTP_POST, std::bind(&WiFiController::handleConnectSTA, this));
    _webServer.on("/status", HTTP_GET, std::bind(&WiFiController::handleGetStatus, this));

    _webServer.on("/save_config", HTTP_POST, std::bind(&WiFiController::handleSaveConfig, this));
    _webServer.on("/start", HTTP_POST, [this]() {
        extern bool isBroadcasting;
        extern uint32_t lastBroadcastTime;
        isBroadcasting = true;
        lastBroadcastTime = millis() - 9999999; // force immediate play
        _webServer.send(200, "text/plain", "Started");
    });
    _webServer.on("/stop", HTTP_POST, [this]() {
        extern bool isBroadcasting;
        isBroadcasting = false;
        _webServer.send(200, "text/plain", "Stopped");
    });

    _webServer.onNotFound(std::bind(&WiFiController::handleNotFound, this));
    _webServer.begin();
}

void WiFiController::handleRoot() {
    if (isCaptivePortalRequest()) {
        _webServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        _webServer.send(302, "text/plain", "");
        return;
    }
    _webServer.send(200, "text/html", PAGE_INDEX);
}

void WiFiController::handleScanResults() {
    int n = WiFi.scanComplete();
    if (n == -2) {
        _webServer.send(200, "application/json", "{\"status\":\"scanning\"}");
    } else if (n == -1) {
        _webServer.send(200, "application/json", "{\"status\":\"failed\"}");
    } else {
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "{";
            json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            json += "\"enc\":" + String(WiFi.encryptionType(i));
            json += "}";
        }
        json += "]";
        WiFi.scanDelete();
        _isScanning = false;
        _webServer.send(200, "application/json", json);
    }
}

void WiFiController::handleConnectSTA() {
    if (_webServer.hasArg("ssid")) {
        String ssid = _webServer.arg("ssid");
        String pass = _webServer.hasArg("pass") ? _webServer.arg("pass") : "";
        connectToSTA(ssid.c_str(), pass.c_str());
        _webServer.send(200, "text/plain", "Connecting...");
    } else {
        _webServer.send(400, "text/plain", "Missing SSID");
    }
}

void WiFiController::handleGetStatus() {
    String json = "{";
    json += "\"sta_status\":" + String(_staStatus) + ",";
    json += "\"sta_ip\":\"" + getSTAIP() + "\",";
    json += "\"ap_ip\":\"" + WiFi.softAPIP().toString() + "\",";
    json += "\"ap_clients\":" + String(getAPClientCount());
    json += "}";
    _webServer.send(200, "application/json", json);
}

void WiFiController::handleSaveConfig() {
    SystemSettings& settings = Config::getInstance().getSettings();
    if (_webServer.hasArg("sequence")) strncpy(settings.sequence, _webServer.arg("sequence").c_str(), sizeof(settings.sequence));
    if (_webServer.hasArg("interval")) settings.interval = _webServer.arg("interval").toInt();
    if (_webServer.hasArg("pitch")) settings.pitch = _webServer.arg("pitch").toInt();
    if (_webServer.hasArg("speed")) settings.speed = _webServer.arg("speed").toInt();

    Config::getInstance().save();
    _webServer.send(200, "text/plain", "Config Saved");
}

void WiFiController::handleNotFound() {
    if (isCaptivePortalRequest()) {
        _webServer.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        _webServer.send(302, "text/plain", "");
        return;
    }
    _webServer.send(404, "text/plain", "Not Found");
}

bool WiFiController::isCaptivePortalRequest() {
    if (!_webServer.hostHeader().equals(WiFi.softAPIP().toString())) {
        return true;
    }
    return false;
}
