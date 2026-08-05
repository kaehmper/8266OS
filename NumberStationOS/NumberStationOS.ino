#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "Config.h"
#include "WiFiController.h"

// Audio includes
#include <AudioOutputI2SNoDAC.h>

AudioOutputI2SNoDAC *out = nullptr;

bool isBroadcasting = false;
uint32_t lastBroadcastTime = 0;

void generateTone(int freq, int durationMs) {
    if (!out) return;
    if (freq == 0) {
        // Rest with abort check
        uint32_t start = millis();
        while (millis() - start < durationMs) {
            yield();
            WiFiController::getInstance().update(); // keep UI alive
            if (!isBroadcasting) return; // instant abort
            delay(1);
        }
        return;
    }

    int sampleRate = 22050;
    int samples = (sampleRate * durationMs) / 1000;
    int halfPeriod = sampleRate / freq / 2;

    int16_t sampleHigh = 16000;
    int16_t sampleLow = -16000;

    for (int i = 0; i < samples; i++) {
        if (i % 1000 == 0) {
            yield();
            WiFiController::getInstance().update();
            if (!isBroadcasting) return; // instant abort
        }
        int16_t currentSample = ((i / halfPeriod) % 2 == 0) ? sampleHigh : sampleLow;
        int16_t stereo[2] = {currentSample, currentSample};
        while (!out->ConsumeSample(stereo)) {
            yield();
        }
    }
}

void playToneSequence(const char* sequence) {
    // User wants 1 low tone, 10 high tone.
    // So if digit is '1', we play a low tone. If digit is '0' (10), we play a high tone.
    // We'll map '1' to 220Hz and '0' to 880Hz.
    // For other digits, we can just interpolate or ignore them.
    for (int i = 0; sequence[i] != 0; i++) {
        if (!isBroadcasting) break;
        char c = sequence[i];
        if (c == '1') {
            generateTone(220, 500);
            generateTone(0, 100);
        } else if (c == '0') {
            generateTone(880, 500);
            generateTone(0, 100);
        } else if (c >= '2' && c <= '9') {
            // interpolate between 220 and 880
            int freq = 220 + (c - '1') * 73;
            generateTone(freq, 500);
            generateTone(0, 100);
        } else if (c == ' ') {
            generateTone(0, 500);
        }
    }
}

const char* getMorseCode(char c) {
    switch (toupper(c)) {
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";
        case '1': return ".----";
        case '2': return "..---";
        case '3': return "...--";
        case '4': return "....-";
        case '5': return ".....";
        case '6': return "-....";
        case '7': return "--...";
        case '8': return "---..";
        case '9': return "----.";
        case '0': return "-----";
        case ' ': return " ";
        default: return "";
    }
}

void playMorseSequence(const char* message, uint16_t wpm) {
    int dotDuration = 1200 / (wpm > 0 ? wpm : 20);
    int freq = 600;

    for (int i = 0; message[i] != 0; i++) {
        if (!isBroadcasting) break;
        const char* morse = getMorseCode(message[i]);
        if (morse[0] == ' ') {
            generateTone(0, dotDuration * 7);
        } else {
            for (int j = 0; morse[j] != 0; j++) {
                if (morse[j] == '.') generateTone(freq, dotDuration);
                else if (morse[j] == '-') generateTone(freq, dotDuration * 3);
                generateTone(0, dotDuration);
            }
            generateTone(0, dotDuration * 2);
        }
    }
}

void playSequence() {
    SystemSettings& settings = Config::getInstance().getSettings();
    out->SetGain(1.0);
    out->SetRate(22050);

    if (settings.broadcastMode == 0) {
        playToneSequence(settings.sequence);
    } else {
        playMorseSequence(settings.morseMessage, settings.wpm);
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nBooting Number Station OS...");

    Config::getInstance().begin();

    out = new AudioOutputI2SNoDAC();
    out->begin();

    WiFiController::getInstance().begin();
    Serial.println("System Ready.");
}

void loop() {
    yield();
    WiFiController::getInstance().update();

    SystemSettings& settings = Config::getInstance().getSettings();
    if (isBroadcasting) {
        uint32_t intervalMs = settings.interval * 60 * 1000;
        if (intervalMs == 0) intervalMs = 60000;

        if (millis() - lastBroadcastTime > intervalMs) {
            playSequence();
            lastBroadcastTime = millis();
        }
    }
}
