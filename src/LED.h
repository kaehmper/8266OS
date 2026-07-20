#ifndef LED_H
#define LED_H

#include <Arduino.h>

enum LEDMode {
    LED_OFF,
    LED_ON,
    LED_BLINK,
    LED_FAST_BLINK,
    LED_BREATHE
};

class LED {
public:
    LED(uint8_t pin, bool activeLow = true);
    void begin();
    void setMode(LEDMode mode);
    LEDMode getMode() const { return _mode; }
    void update();

private:
    uint8_t _pin;
    bool _activeLow;
    LEDMode _mode;
    uint32_t _lastToggleTime;
    bool _blinkState;
};

#endif // LED_H
