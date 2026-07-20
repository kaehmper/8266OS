#ifndef OS_UI_H
#define OS_UI_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "Button.h"
#include "FlappyGame.h"

enum UIScreen {
    SCR_MENU,
    SCR_STATUS,
    SCR_WIFI_SCAN,
    SCR_CHAT_STATS,
    SCR_GAME,
    SCR_OPTIONS,
    SCR_ABOUT
};

class OSUI {
public:
    static OSUI& getInstance() {
        static OSUI instance;
        return instance;
    }

    void begin(Adafruit_SSD1306* display);
    void update(Button& button);
    void draw(float holdProgress);

    // Screen saver control
    void resetSleepTimer();
    bool isAsleep() const { return _isAsleep; }
    void wakeUp();

private:
    OSUI() : _display(nullptr), _currentScreen(SCR_MENU), _menuIndex(0),
             _optionsIndex(0), _wifiScanIndex(0), _isAsleep(false),
             _lastActivityTime(0) {}

    Adafruit_SSD1306* _display;
    UIScreen _currentScreen;

    // Menu selections
    uint8_t _menuIndex;
    uint8_t _optionsIndex;
    uint8_t _wifiScanIndex;

    // Screensaver
    bool _isAsleep;
    uint32_t _lastActivityTime;

    // Mini-game instance
    FlappyGame _game;

    // Helper draw functions
    void drawStatusBar();
    void drawMenu();
    void drawStatusScreen();
    void drawWiFiScanScreen();
    void drawChatStatsScreen();
    void drawOptionsScreen();
    void drawAboutScreen();
    void drawProgressBar(float progress);

    static const uint8_t MENU_COUNT = 6;
    static const char* MENU_ITEMS[MENU_COUNT];

    static const uint8_t OPTIONS_COUNT = 6;
    static const char* OPTIONS_ITEMS[OPTIONS_COUNT];
};

#endif // OS_UI_H
