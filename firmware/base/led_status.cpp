#include "led_status.h"

#define BREATH_INTERVAL_MS 20
#define BLINK_INTERVAL_MS  500

void StatusLed::init(uint8_t pin) {
    _pixel = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);
    _pixel.begin();
    _pixel.setBrightness(LED_BRIGHTNESS);
    _pixel.clear();
    _pixel.show();
}

void StatusLed::setPattern(LedPattern pattern) {
    _pattern = pattern;
    _breathValue = 0;
    _breathUp = true;
}

void StatusLed::update() {
    uint32_t now = millis();

    // Handle flash override
    if (_flashActive) {
        if (now >= _flashEnd) {
            _flashActive = false;
            _pattern = _preFlashPattern;
        } else {
            return; // Flash color already set
        }
    }

    switch (_pattern) {
        case LED_OFF:
            _pixel.clear();
            break;

        case LED_SOLID_GREEN:
            _pixel.setPixelColor(0, _pixel.Color(0, 255, 0));
            break;

        case LED_SOLID_BLUE:
            _pixel.setPixelColor(0, _pixel.Color(0, 0, 255));
            break;

        case LED_SOLID_PURPLE:
            _pixel.setPixelColor(0, _pixel.Color(128, 0, 255));
            break;

        case LED_BREATHING_RED:
            if (now - _lastUpdate >= BREATH_INTERVAL_MS) {
                _lastUpdate = now;
                if (_breathUp) {
                    _breathValue += 5;
                    if (_breathValue >= 255) { _breathValue = 255; _breathUp = false; }
                } else {
                    _breathValue -= 5;
                    if (_breathValue <= 10) { _breathValue = 10; _breathUp = true; }
                }
                _pixel.setPixelColor(0, _pixel.Color(_breathValue, 0, 0));
            }
            break;

        case LED_BREATHING_YELLOW:
            if (now - _lastUpdate >= BREATH_INTERVAL_MS) {
                _lastUpdate = now;
                if (_breathUp) {
                    _breathValue += 5;
                    if (_breathValue >= 255) { _breathValue = 255; _breathUp = false; }
                } else {
                    _breathValue -= 5;
                    if (_breathValue <= 10) { _breathValue = 10; _breathUp = true; }
                }
                _pixel.setPixelColor(0, _pixel.Color(_breathValue, _breathValue, 0));
            }
            break;

        case LED_BLINK_WHITE:
            if (now - _lastUpdate >= BLINK_INTERVAL_MS) {
                _lastUpdate = now;
                _breathUp = !_breathUp;
                if (_breathUp) {
                    _pixel.setPixelColor(0, _pixel.Color(255, 255, 255));
                } else {
                    _pixel.clear();
                }
            }
            break;
    }

    _pixel.show();
}

void StatusLed::flash(uint32_t color, uint16_t durationMs) {
    if (!_flashActive) {
        _preFlashPattern = _pattern;
    }
    _flashActive = true;
    _flashEnd = millis() + durationMs;
    _pixel.setPixelColor(0, color);
    _pixel.show();
}
