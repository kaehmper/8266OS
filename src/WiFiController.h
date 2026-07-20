#ifndef WIFI_CONTROLLER_H
#define WIFI_CONTROLLER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <vector>

struct ScannedNetwork {
    char ssid[33];
    int32_t rssi;
    uint8_t encryptionType;
};

class WiFiController {
public:
    static WiFiController& getInstance() {
        static WiFiController instance;
        return instance;
    }

    void begin();
    void update();

    void startAP();
    void connectToSTA(const char* ssid, const char* pass);
    void disconnectSTA();

    // Async Network Scanning
    void startScan();
    bool isScanning() const { return _isScanning; }
    const std::vector<ScannedNetwork>& getScannedNetworks() const { return _scannedNetworks; }

    // Info
    uint8_t getAPClientCount() const;
    wl_status_t getSTAStatus() const { return _staStatus; }
    String getSTAIP() const { return WiFi.localIP().toString(); }
    String getAPIP() const { return WiFi.softAPIP().toString(); }

private:
    WiFiController() : _isScanning(false), _lastScanTime(0), _staStatus(WL_DISCONNECTED), _staConnecting(false), _staConnectStartTime(0) {}

    DNSServer _dnsServer;
    ESP8266WebServer _webServer;

    bool _isScanning;
    uint32_t _lastScanTime;
    std::vector<ScannedNetwork> _scannedNetworks;

    wl_status_t _staStatus;
    bool _staConnecting;
    uint32_t _staConnectStartTime;

    void setupWebServer();
    void handleRoot();
    void handleGetMessages();
    void handleSendMessage();
    void handleScanResults();
    void handleConnectSTA();
    void handleGetStatus();
    void handleSaveConfig();
    void handleNotFound();

    bool isCaptivePortalRequest();
};

#endif // WIFI_CONTROLLER_H
