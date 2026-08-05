#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "WiFiController.h"

// Audio includes
#include <AudioOutputI2SNoDAC.h>
#include <AudioGeneratorWAV.h>
#include <ESP8266SAM_ES.h>

AudioOutputI2SNoDAC *out = nullptr;
ESP8266SAM_ES *sam = nullptr;

bool isBroadcasting = false;
uint32_t lastBroadcastTime = 0;

void generateTone(int freq, int durationMs) {
    if (!out) return;

    // Simple square wave generation for chimes
    int sampleRate = 22050;
    int samples = (sampleRate * durationMs) / 1000;
    int halfPeriod = sampleRate / freq / 2;

    int16_t sampleHigh = 32000;
    int16_t sampleLow = -32000;

    for (int i = 0; i < samples; i++) {
        // Feed the watchdog to prevent crashes during long tones
        if (i % 1000 == 0) yield();

        int16_t currentSample = ((i / halfPeriod) % 2 == 0) ? sampleHigh : sampleLow;
        // out->ConsumeSample sends left and right channel (we send same to both)
        int16_t stereo[2] = {currentSample, currentSample};

        // Block until sample is consumed
        while (!out->ConsumeSample(stereo)) {
            yield();
        }
    }
}

void playChime() {
    out->SetGain(0.2); // Lower volume for chime
    out->SetRate(22050);
    generateTone(440, 500); // A4
    delay(100);
    generateTone(554, 500); // C#5
    delay(100);
    generateTone(659, 1000); // E5
    delay(500);
}

void playSequence() {
    SystemSettings& settings = Config::getInstance().getSettings();
    if (sam && out) {
        playChime();

        out->SetGain(1.0); // Full volume for voice
        sam->SetPitch(settings.pitch);
        sam->SetSpeed(settings.speed);
        sam->Say(out, settings.sequence);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nBooting Number Station OS...");

    // Initialize Persistent Configuration
    Config::getInstance().begin();

    // Setup Audio
    // I2S No DAC uses GPIO3 (RX pin) for output by default
    out = new AudioOutputI2SNoDAC();
    out->begin();

    sam = new ESP8266SAM_ES();

    // Initialize WiFi Access Point + Web Server + Captive Portal
    WiFiController::getInstance().begin();

    Serial.println("System Ready.");
}

void loop() {
    yield();
    WiFiController::getInstance().update();

    SystemSettings& settings = Config::getInstance().getSettings();
    if (isBroadcasting) {
        uint32_t intervalMs = settings.interval * 60 * 1000;
        if (intervalMs == 0) intervalMs = 60000; // minimum 1 min

        if (millis() - lastBroadcastTime > intervalMs) {
            playSequence();
            lastBroadcastTime = millis();
        }
    }
}
