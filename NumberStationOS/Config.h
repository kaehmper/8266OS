#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct SystemSettings {
    uint32_t magic;
    char apSSID[33];
    char apPass[65];
    char staSSID[33];
    char staPass[65];
    char sequence[128]; // Max 128 chars of sequence
    uint8_t interval; // Interval in minutes
    uint8_t contrast;
    uint8_t ledSetting;
    uint8_t pitch;
    uint8_t speed;
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
