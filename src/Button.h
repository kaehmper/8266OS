#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
public:
    Button(uint8_t pin, bool activeLow = true, uint32_t debounceMs = 50);
    void begin();
    void update();

    // Event checking (clears the flag on read)
    bool wasClicked();
    bool wasLongPressed();

    // Real-time state
    bool isDown() const { return _isDown; }
    uint32_t getHoldDuration() const;
    float getHoldProgress() const; // Returns 0.0 to 1.0 based on LONG_PRESS_MS

    static const uint32_t LONG_PRESS_MS = 600;

private:
    uint8_t _pin;
    bool _activeLow;
    uint32_t _debounceMs;

    bool _lastRawState;
    bool _debouncedState;
    uint32_t _lastDebounceTime;

    bool _isDown;
    uint32_t _pressStartTime;
    bool _longPressTriggered;

    bool _clickedPending;
    bool _longPressedPending;
};

#endif // BUTTON_H
