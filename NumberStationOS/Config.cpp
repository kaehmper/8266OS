#include "Config.h"
#include <EEPROM.h>

void Config::begin() {
    EEPROM.begin(512);
    load();
}

void Config::load() {
    uint8_t* ptr = (uint8_t*)&_settings;
    for (size_t i = 0; i < sizeof(SystemSettings); i++) {
        ptr[i] = EEPROM.read(i);
    }

    if (_settings.magic != MAGIC_VAL) {
        resetToDefaults();
    }
}

void Config::save() {
    _settings.magic = MAGIC_VAL;
    const uint8_t* ptr = (const uint8_t*)&_settings;
    for (size_t i = 0; i < sizeof(SystemSettings); i++) {
        EEPROM.write(i, ptr[i]);
    }
    EEPROM.commit();
}

void Config::resetToDefaults() {
    memset(&_settings, 0, sizeof(SystemSettings));
    _settings.magic = MAGIC_VAL;
    strcpy(_settings.apSSID, "");
    strcpy(_settings.apPass, "");
    strcpy(_settings.staSSID, "");
    strcpy(_settings.staPass, "");
    strcpy(_settings.sequence, "5 4 3 2 1");
    _settings.contrast = 127;
    _settings.ledSetting = 3; // Breathe by default
    _settings.interval = 1;
    _settings.pitch = 64;
    _settings.speed = 72;
    save();
}
