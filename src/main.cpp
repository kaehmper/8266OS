#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Button.h"
#include "LED.h"
#include "Config.h"
#include "ChatManager.h"
#include "WiFiController.h"
#include "OSUI.h"

// 1. Hardware Pin Definitions (configured in platformio.ini)
// BUTTON_PIN = 1  (GPIO1 / TX)
// LED_PIN    = 3  (GPIO3 / RX)
// SDA_PIN    = 0  (GPIO0)
// SCL_PIN    = 2  (GPIO2)

// 2. Global Object Instances
Adafruit_SSD1306 display(128, 64, &Wire, -1);
Button systemButton(BUTTON_PIN, true, 40); // Active Low, 40ms debounce
LED systemLed(LED_PIN, true);             // Active Low LED

// 3. Global Callback Implementations
void updateOLEDContrast(uint8_t contrast) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(contrast);
}

// 4. Setup
void setup() {
    // Disable Serial TX/RX so they don't interfere with the Button/LED pins
    // (GPIO1 and GPIO3 are shared with Serial)
    Serial.end();

    // Initialize Persistent Configuration
    Config::getInstance().begin();
    SystemSettings& settings = Config::getInstance().getSettings();

    // Initialize Button and LED
    systemButton.begin();
    systemLed.begin();
    systemLed.setMode((LEDMode)settings.ledSetting);

    // Initialize I2C Bus for SSD1306 OLED screen
    Wire.begin(SDA_PIN, SCL_PIN);

    // Initialize SSD1306 Display (I2C address is usually 0x3C)
    if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        display.clearDisplay();
        updateOLEDContrast(settings.contrast);

        // Render beautiful booting splash screen
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(18, 14);
        display.print("LOCAL CHAT OS");
        display.setCursor(34, 28);
        display.print("Booting...");

        // Animate a sleek loading bar
        for (int16_t w = 0; w <= 80; w += 4) {
            display.drawRect(24, 42, 80, 6, SSD1306_WHITE);
            display.fillRect(24, 42, w, 6, SSD1306_WHITE);
            display.display();
            delay(40);
        }
        delay(150);
    }

    // Initialize Chat Memory
    ChatManager::getInstance().begin();
    ChatManager::getInstance().addMessage("System", "Welcome to Local Chat!");

    // Initialize WiFi Access Point + Web Server + Captive Portal
    WiFiController::getInstance().begin();

    // Initialize Modern OS UI
    OSUI::getInstance().begin(&display);
}

// 5. Main Multitasking Non-Blocking Loop
void loop() {
    // Let the ESP8266 background tasks run smoothly
    yield();

    // Update physical peripherals
    systemButton.update();
    systemLed.update();

    // Update background tasks
    WiFiController::getInstance().update();

    // Update UI State Machine
    OSUI::getInstance().update(systemButton);

    // Only render the OLED screen if the system is not asleep
    if (!OSUI::getInstance().isAsleep()) {
        float holdProgress = systemButton.getHoldProgress();
        OSUI::getInstance().draw(holdProgress);
    }

    // Microscopic delay to protect CPU temperature and keep loop responsive
    delay(1);
}
