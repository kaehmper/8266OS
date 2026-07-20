#include "OSUI.h"
#include "Config.h"
#include "LED.h"
#include "WiFiController.h"
#include "ChatManager.h"

extern LED systemLed;
extern void updateOLEDContrast(uint8_t contrast);

const char* OSUI::MENU_ITEMS[MENU_COUNT] = {
    "1. System Status",
    "2. WiFi Scanner",
    "3. Local Chat Stats",
    "4. Play Flappy ESP",
    "5. System Options",
    "6. About System"
};

const char* OSUI::OPTIONS_ITEMS[OPTIONS_COUNT] = {
    "LED Mode",
    "OLED Contrast",
    "Screen Saver",
    "Clear Local Chat",
    "Factory Reset",
    "<- Back to Menu"
};

void OSUI::begin(Adafruit_SSD1306* display) {
    _display = display;
    _currentScreen = SCR_MENU;
    _menuIndex = 0;
    _optionsIndex = 0;
    _wifiScanIndex = 0;
    _isAsleep = false;
    _lastActivityTime = millis();
}

void OSUI::resetSleepTimer() {
    _lastActivityTime = millis();
}

void OSUI::wakeUp() {
    _isAsleep = false;
    resetSleepTimer();
    _display->ssd1306_command(SSD1306_DISPLAYON);
}

void OSUI::update(Button& button) {
    uint32_t now = millis();
    SystemSettings& settings = Config::getInstance().getSettings();

    // Handle Screensaver Timeout
    if (!_isAsleep && settings.screenSaverMins > 0) {
        if (now - _lastActivityTime > (uint32_t)settings.screenSaverMins * 60 * 1000) {
            _isAsleep = true;
            _display->ssd1306_command(SSD1306_DISPLAYOFF);
        }
    }

    // Capture physical interaction
    bool clicked = button.wasClicked();
    bool longPressed = button.wasLongPressed();

    if (clicked || longPressed || button.isDown()) {
        if (_isAsleep) {
            wakeUp();
            return; // Consume the button event on wakeup
        }
        resetSleepTimer();
    }

    if (_isAsleep) {
        return;
    }

    // If we are playing the mini-game, give it exclusive input control
    if (_currentScreen == SCR_GAME) {
        _game.update(clicked, longPressed);
        if (_game.shouldExit()) {
            _currentScreen = SCR_MENU;
            _game.clearExitFlag();
        }
        return;
    }

    // Normal menu navigation
    if (clicked) {
        switch (_currentScreen) {
            case SCR_MENU:
                _menuIndex = (_menuIndex + 1) % MENU_COUNT;
                break;

            case SCR_OPTIONS:
                _optionsIndex = (_optionsIndex + 1) % OPTIONS_COUNT;
                break;

            case SCR_WIFI_SCAN: {
                uint8_t count = WiFiController::getInstance().getScannedNetworks().size();
                // We add 1 for the "Back" option
                _wifiScanIndex = (_wifiScanIndex + 1) % (count + 1);
                break;
            }

            // Simple screens - click to go back to main menu
            case SCR_STATUS:
            case SCR_CHAT_STATS:
            case SCR_ABOUT:
                _currentScreen = SCR_MENU;
                break;

            default:
                break;
        }
    }

    if (longPressed) {
        switch (_currentScreen) {
            case SCR_MENU:
                switch (_menuIndex) {
                    case 0: _currentScreen = SCR_STATUS; break;
                    case 1:
                        _currentScreen = SCR_WIFI_SCAN;
                        _wifiScanIndex = 0;
                        WiFiController::getInstance().startScan();
                        break;
                    case 2: _currentScreen = SCR_CHAT_STATS; break;
                    case 3:
                        _currentScreen = SCR_GAME;
                        _game.reset();
                        break;
                    case 4:
                        _currentScreen = SCR_OPTIONS;
                        _optionsIndex = 0;
                        break;
                    case 5: _currentScreen = SCR_ABOUT; break;
                }
                break;

            case SCR_STATUS:
            case SCR_CHAT_STATS:
            case SCR_ABOUT:
                _currentScreen = SCR_MENU;
                break;

            case SCR_WIFI_SCAN: {
                const auto& nets = WiFiController::getInstance().getScannedNetworks();
                if (_wifiScanIndex == nets.size() || nets.empty()) {
                    // Selected "Back" or empty list
                    _currentScreen = SCR_MENU;
                } else {
                    // Try to connect to selected WiFi
                    WiFiController::getInstance().connectToSTA(nets[_wifiScanIndex].ssid, "");
                    _currentScreen = SCR_STATUS; // Go to status to show connection status
                }
                break;
            }

            case SCR_OPTIONS:
                switch (_optionsIndex) {
                    case 0: // Toggle LED Mode
                        settings.ledSetting = (settings.ledSetting + 1) % 4;
                        systemLed.setMode((LEDMode)settings.ledSetting);
                        Config::getInstance().save();
                        break;

                    case 1: // Toggle OLED Contrast
                        if (settings.contrast == 64) settings.contrast = 127;
                        else if (settings.contrast == 127) settings.contrast = 255;
                        else settings.contrast = 64;
                        updateOLEDContrast(settings.contrast);
                        Config::getInstance().save();
                        break;

                    case 2: // Toggle Screensaver minutes
                        if (settings.screenSaverMins == 0) settings.screenSaverMins = 1;
                        else if (settings.screenSaverMins == 1) settings.screenSaverMins = 2;
                        else if (settings.screenSaverMins == 2) settings.screenSaverMins = 5;
                        else settings.screenSaverMins = 0;
                        Config::getInstance().save();
                        break;

                    case 3: // Clear local chat history
                        ChatManager::getInstance().clear();
                        break;

                    case 4: // Factory Reset
                        Config::getInstance().resetToDefaults();
                        updateOLEDContrast(settings.contrast);
                        systemLed.setMode((LEDMode)settings.ledSetting);
                        _currentScreen = SCR_MENU;
                        break;

                    case 5: // Back
                        _currentScreen = SCR_MENU;
                        break;
                }
                break;

            default:
                break;
        }
    }
}

void OSUI::draw(float holdProgress) {
    if (_isAsleep) {
        return;
    }

    if (_currentScreen == SCR_GAME) {
        _game.draw(*_display);
        _display->display();
        return;
    }

    _display->clearDisplay();

    // 1. Draw Global Status Bar
    drawStatusBar();

    // 2. Draw Main Screen Contents
    switch (_currentScreen) {
        case SCR_MENU:
            drawMenu();
            break;
        case SCR_STATUS:
            drawStatusScreen();
            break;
        case SCR_WIFI_SCAN:
            drawWiFiScanScreen();
            break;
        case SCR_CHAT_STATS:
            drawChatStatsScreen();
            break;
        case SCR_OPTIONS:
            drawOptionsScreen();
            break;
        case SCR_ABOUT:
            drawAboutScreen();
            break;
        default:
            break;
    }

    // 3. Draw hold progress feedback bar at the bottom
    drawProgressBar(holdProgress);

    _display->display();
}

void OSUI::drawStatusBar() {
    _display->setTextSize(1);
    _display->setTextColor(SSD1306_WHITE);
    _display->setCursor(2, 0);

    // AP connection and mode
    uint8_t clientCount = WiFiController::getInstance().getAPClientCount();
    _display->print("[AP] C:");
    _display->print(clientCount);

    // STA connection status icon
    _display->setCursor(62, 0);
    wl_status_t status = WiFiController::getInstance().getSTAStatus();
    if (status == WL_CONNECTED) {
        _display->print("S:OK");
    } else if (status == WL_IDLE_STATUS || WiFi.status() == WL_DISCONNECTED) {
        _display->print("S:--");
    } else {
        _display->print("S:..");
    }

    // Uptime (HH:MM:SS format)
    uint32_t uptimeSec = millis() / 1000;
    uint32_t mins = (uptimeSec % 3600) / 60;
    uint32_t secs = uptimeSec % 60;

    char uptimeStr[10];
    snprintf(uptimeStr, sizeof(uptimeStr), "%02d:%02d", mins, secs);
    _display->setCursor(96, 0);
    _display->print(uptimeStr);

    // Thin separator line
    _display->drawFastHLine(0, 9, 128, SSD1306_WHITE);
}

void OSUI::drawMenu() {
    _display->setCursor(2, 12);
    _display->setTextSize(1);
    _display->print("--- LOCAL OS MENU ---");

    // We can display 3 menu items at once
    // Calculate display sliding window
    uint8_t startIdx = 0;
    if (_menuIndex >= 3) {
        startIdx = _menuIndex - 2;
        if (startIdx + 3 > MENU_COUNT) {
            startIdx = MENU_COUNT - 3;
        }
    }

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t itemIdx = startIdx + i;
        int16_t yPos = 24 + (i * 10);
        _display->setCursor(4, yPos);

        if (itemIdx == _menuIndex) {
            _display->print("> ");
        } else {
            _display->print("  ");
        }
        _display->print(MENU_ITEMS[itemIdx]);
    }
}

void OSUI::drawStatusScreen() {
    _display->setCursor(2, 12);
    _display->print("--- SYSTEM STATUS ---");

    _display->setCursor(4, 22);
    _display->print("AP IP:  192.168.4.1");

    _display->setCursor(4, 32);
    wl_status_t status = WiFiController::getInstance().getSTAStatus();
    if (status == WL_CONNECTED) {
        _display->print("STA IP: ");
        _display->print(WiFiController::getInstance().getSTAIP());
    } else if (status == WL_CONNECT_FAILED) {
        _display->print("STA:   Failed");
    } else {
        _display->print("STA:   Not Connected");
    }

    _display->setCursor(4, 42);
    _display->print("Heap:  ");
    _display->print(ESP.getFreeHeap() / 1024);
    _display->print(" KB");

    _display->setCursor(4, 52);
    _display->print("Clients connected: ");
    _display->print(WiFiController::getInstance().getAPClientCount());
}

void OSUI::drawWiFiScanScreen() {
    _display->setCursor(2, 12);
    _display->print("--- WIFI SCANNER ---");

    const auto& nets = WiFiController::getInstance().getScannedNetworks();

    if (WiFiController::getInstance().isScanning()) {
        _display->setCursor(4, 30);
        _display->print("Scanning networks...");
        return;
    }

    if (nets.empty()) {
        _display->setCursor(4, 24);
        _display->print("No networks found.");
        _display->setCursor(4, 40);
        if (_wifiScanIndex == 0) _display->print("> <- Back to Menu");
        else _display->print("  <- Back to Menu");
        return;
    }

    uint8_t totalItems = nets.size() + 1; // Networks + "Back" option

    // We can display 3 networks at once
    uint8_t startIdx = 0;
    if (_wifiScanIndex >= 3) {
        startIdx = _wifiScanIndex - 2;
        if (startIdx + 3 > totalItems) {
            startIdx = totalItems - 3;
        }
    }

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t itemIdx = startIdx + i;
        int16_t yPos = 22 + (i * 10);
        _display->setCursor(4, yPos);

        if (itemIdx == _wifiScanIndex) {
            _display->print("> ");
        } else {
            _display->print("  ");
        }

        if (itemIdx == nets.size()) {
            _display->print("<- Back to Menu");
        } else {
            // Trim SSID if too long for screen
            String ssid = nets[itemIdx].ssid;
            if (ssid.length() > 10) {
                ssid = ssid.substring(0, 10) + "..";
            }
            _display->print(ssid);
            _display->print(" (");
            _display->print(nets[itemIdx].rssi);
            _display->print(")");
        }
    }
}

void OSUI::drawChatStatsScreen() {
    _display->setCursor(2, 12);
    _display->print("--- CHAT ROOM STATS --");

    uint32_t count = ChatManager::getInstance().getMessageCount();
    _display->setCursor(4, 22);
    _display->print("Total Messages: ");
    _display->print(count);

    _display->setCursor(4, 32);
    _display->print("Last message received:");

    const auto& msgs = ChatManager::getInstance().getMessages();
    if (msgs.empty()) {
        _display->setCursor(4, 44);
        _display->print("(No messages yet)");
    } else {
        const ChatMessage& lastMsg = msgs.back();
        // Draw sender and snippet
        _display->setCursor(4, 44);
        _display->setTextColor(SSD1306_WHITE);
        _display->print("[");
        _display->print(lastMsg.nickname);
        _display->print("]: ");

        // Render snippet
        _display->setCursor(4, 54);
        String textSnippet = String(lastMsg.text);
        if (textSnippet.length() > 20) {
            textSnippet = textSnippet.substring(0, 17) + "...";
        }
        _display->print(textSnippet);
    }
}

void OSUI::drawOptionsScreen() {
    _display->setCursor(2, 12);
    _display->print("--- SYSTEM OPTIONS ---");

    SystemSettings& settings = Config::getInstance().getSettings();

    // We can display 3 options at once
    uint8_t startIdx = 0;
    if (_optionsIndex >= 3) {
        startIdx = _optionsIndex - 2;
        if (startIdx + 3 > OPTIONS_COUNT) {
            startIdx = OPTIONS_COUNT - 3;
        }
    }

    for (uint8_t i = 0; i < 3; i++) {
        uint8_t itemIdx = startIdx + i;
        int16_t yPos = 22 + (i * 10);
        _display->setCursor(4, yPos);

        if (itemIdx == _optionsIndex) {
            _display->print("> ");
        } else {
            _display->print("  ");
        }

        _display->print(OPTIONS_ITEMS[itemIdx]);

        // Add setting value descriptors
        if (itemIdx == 0) {
            _display->print(": ");
            if (settings.ledSetting == 0) _display->print("Off");
            else if (settings.ledSetting == 1) _display->print("On");
            else if (settings.ledSetting == 2) _display->print("Blink");
            else _display->print("Pulse");
        } else if (itemIdx == 1) {
            _display->print(": ");
            if (settings.contrast == 64) _display->print("Low");
            else if (settings.contrast == 127) _display->print("Med");
            else _display->print("High");
        } else if (itemIdx == 2) {
            _display->print(": ");
            if (settings.screenSaverMins == 0) _display->print("Never");
            else {
                _display->print(settings.screenSaverMins);
                _display->print("m");
            }
        }
    }
}

void OSUI::drawAboutScreen() {
    _display->setCursor(2, 12);
    _display->print("--- ABOUT LOCAL OS ---");

    _display->setCursor(4, 24);
    _display->print("Author: Creative Eng");

    _display->setCursor(4, 34);
    _display->print("Platform: ESP8266-01");

    _display->setCursor(4, 44);
    _display->print("Flash: 1MB  RAM: 80KB");

    _display->setCursor(4, 54);
    _display->print("Hold button to exit.");
}

void OSUI::drawProgressBar(float progress) {
    if (progress <= 0.05f) return;

    // Draw a modern bottom progress bar to indicate hold status
    int16_t barWidth = (int16_t)(progress * 128.0f);
    _display->fillRect(0, 61, barWidth, 3, SSD1306_WHITE);
}
