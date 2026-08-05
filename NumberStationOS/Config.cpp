#include "Config.h"
#include <EEPROM.h>

void Config::begin() {
    EEPROM.begin(sizeof(SystemSettings));
    load();
}

void Config::load() {
    EEPROM.get(0, _settings);

    // If magic byte doesn't match, eeprom is uninitialized or corrupted
    if (_settings.magic != MAGIC_VAL) {
        resetToDefaults();
        save();
    }
}

void Config::save() {
    _settings.magic = MAGIC_VAL;
    EEPROM.put(0, _settings);
    EEPROM.commit();
}

void Config::resetToDefaults() {
    _settings.magic = MAGIC_VAL;
    strcpy(_settings.apSSID, "");
    strcpy(_settings.apPass, "");
    strcpy(_settings.staSSID, "");
    strcpy(_settings.staPass, "");
    strcpy(_settings.sequence, "5 4 3 2 1");
    strcpy(_settings.morseMessage, "HELLO");
    strcpy(_settings.ttsMessage, "SYSTEM READY");
    _settings.interval = 1;
    _settings.broadcastMode = 0;
    _settings.wpm = 20;
    _settings.pitch = 64;
    _settings.speed = 72;
}
