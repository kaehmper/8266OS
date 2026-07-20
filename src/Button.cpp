#include "Button.h"

Button::Button(uint8_t pin, bool activeLow, uint32_t debounceMs)
    : _pin(pin), _activeLow(activeLow), _debounceMs(debounceMs),
      _lastRawState(false), _debouncedState(false), _lastDebounceTime(0),
      _isDown(false), _pressStartTime(0), _longPressTriggered(false),
      _clickedPending(false), _longPressedPending(false) {}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);
    // Initial read
    bool initialRead = (digitalRead(_pin) == LOW);
    _lastRawState = initialRead;
    _debouncedState = initialRead;
    _isDown = initialRead;
}

void Button::update() {
    bool rawState = (digitalRead(_pin) == (_activeLow ? LOW : HIGH));
    uint32_t now = millis();

    // Debouncing
    if (rawState != _lastRawState) {
        _lastDebounceTime = now;
        _lastRawState = rawState;
    }

    if ((now - _lastDebounceTime) > _debounceMs) {
        if (rawState != _debouncedState) {
            _debouncedState = rawState;

            if (_debouncedState) {
                // Button was pressed down
                _isDown = true;
                _pressStartTime = now;
                _longPressTriggered = false;
            } else {
                // Button was released
                _isDown = false;
                uint32_t holdTime = now - _pressStartTime;

                if (!_longPressTriggered) {
                    if (holdTime < LONG_PRESS_MS && holdTime > 20) {
                        _clickedPending = true;
                    }
                }
                _longPressTriggered = false;
            }
        }
    }

    // Long press detection while holding down
    if (_isDown && !_longPressTriggered) {
        if ((now - _pressStartTime) >= LONG_PRESS_MS) {
            _longPressTriggered = true;
            _longPressedPending = true;
        }
    }
}

bool Button::wasClicked() {
    if (_clickedPending) {
        _clickedPending = false;
        return true;
    }
    return false;
}

bool Button::wasLongPressed() {
    if (_longPressedPending) {
        _longPressedPending = false;
        return true;
    }
    return false;
}

uint32_t Button::getHoldDuration() const {
    if (!_isDown) return 0;
    return millis() - _pressStartTime;
}

float Button::getHoldProgress() const {
    if (!_isDown) return 0.0f;
    uint32_t duration = millis() - _pressStartTime;
    if (duration >= LONG_PRESS_MS) return 1.0f;
    return (float)duration / (float)LONG_PRESS_MS;
}
