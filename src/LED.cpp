#include "LED.h"
#include <math.h>

LED::LED(uint8_t pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow), _mode(LED_OFF), _lastToggleTime(0), _blinkState(false) {}

void LED::begin() {
    pinMode(_pin, OUTPUT);
    setMode(LED_OFF);
}

void LED::setMode(LEDMode mode) {
    if (_mode == mode) return;
    _mode = mode;
    _lastToggleTime = millis();
    _blinkState = false;

    // Reset pin states immediately for static modes
    if (_mode == LED_OFF) {
        digitalWrite(_pin, _activeLow ? HIGH : LOW);
    } else if (_mode == LED_ON) {
        digitalWrite(_pin, _activeLow ? LOW : HIGH);
    }
}

void LED::update() {
    uint32_t now = millis();

    switch (_mode) {
        case LED_OFF:
            // Handled in setMode, but ensure it's off
            break;

        case LED_ON:
            // Handled in setMode
            break;

        case LED_BLINK: {
            if (now - _lastToggleTime >= 500) {
                _lastToggleTime = now;
                _blinkState = !_blinkState;
                digitalWrite(_pin, (_blinkState ^ _activeLow) ? HIGH : LOW);
            }
            break;
        }

        case LED_FAST_BLINK: {
            if (now - _lastToggleTime >= 150) {
                _lastToggleTime = now;
                _blinkState = !_blinkState;
                digitalWrite(_pin, (_blinkState ^ _activeLow) ? HIGH : LOW);
            }
            break;
        }

        case LED_BREATHE: {
            // Breathe uses a sine wave over a 2000ms period
            // Calculate a value between 0 and 1023
            float phase = (float)(now % 2000) / 2000.0f * 2.0f * M_PI;
            float intensity = (sinf(phase - M_PI / 2.0f) + 1.0f) / 2.0f; // 0.0 to 1.0
            uint16_t pwmVal = (uint16_t)(intensity * 1023.0f);

            if (_activeLow) {
                analogWrite(_pin, 1023 - pwmVal);
            } else {
                analogWrite(_pin, pwmVal);
            }
            break;
        }
    }
}
