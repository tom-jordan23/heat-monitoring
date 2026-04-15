#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Status LED patterns for node state indication.
// Uses a single addressable RGB NeoPixel.

#define DEFAULT_LED_PIN 48
#define LED_BRIGHTNESS  50    // 0-255, keep low for power savings

enum LedPattern {
    LED_OFF,
    LED_SOLID_GREEN,      // Normal operation
    LED_SOLID_BLUE,       // Initializing / registering
    LED_BREATHING_RED,    // Active event / alert
    LED_BREATHING_YELLOW, // Warning state
    LED_BLINK_WHITE,      // LoRa TX/RX activity
    LED_SOLID_PURPLE      // ML inference active
};

class StatusLed {
public:
    // Initialize the NeoPixel
    void init(uint8_t pin = DEFAULT_LED_PIN);

    // Set the current pattern. Call update() periodically to animate.
    void setPattern(LedPattern pattern);

    // Must be called from loop() to animate breathing/blink patterns.
    // Non-blocking — uses millis() internally.
    void update();

    // Momentary flash for activity indication (e.g., LoRa packet sent).
    // Overrides current pattern briefly, then reverts.
    void flash(uint32_t color, uint16_t durationMs = 100);

private:
    Adafruit_NeoPixel _pixel;
    LedPattern _pattern = LED_OFF;
    uint32_t _lastUpdate = 0;
    uint8_t _breathValue = 0;
    bool _breathUp = true;
    bool _flashActive = false;
    uint32_t _flashEnd = 0;
    LedPattern _preFlashPattern = LED_OFF;
};

#endif // LED_STATUS_H
