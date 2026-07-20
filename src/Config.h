#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct SystemSettings {
    uint16_t magic;          // Magic number to verify initialization
    char wifiSSID[33];       // Max 32 chars + null
    char wifiPass[65];       // Max 64 chars + null
    uint8_t contrast;        // OLED contrast (0 - 255)
    uint8_t ledSetting;      // 0: Off, 1: On, 2: Blink, 3: Breathe (default)
    uint32_t gameHighScore;  // High score of Flappy ESP
    uint8_t screenSaverMins; // Screen saver idle timeout (0 to disable)
};

class Config {
public:
    static Config& getInstance() {
        static Config instance;
        return instance;
    }

    void begin();
    void load();
    void save();
    void resetToDefaults();

    SystemSettings& getSettings() { return _settings; }

private:
    Config() {}
    SystemSettings _settings;

    static const uint16_t MAGIC_VAL = 0xDEAC;
};

#endif // CONFIG_H
