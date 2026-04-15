#include "power_mgmt.h"

// Voltage divider ratio — adjust based on actual resistor values on PCB.
// Forest-Guard uses a 100K/100K divider, so ratio = 2.0
#define VDIV_RATIO 2.0f

// ESP32-S3 ADC reference voltage
#define ADC_REF_VOLTAGE 3.3f
#define ADC_RESOLUTION  4095.0f

void PowerManager::init(uint8_t vbatPin) {
    _vbatPin = vbatPin;
    analogReadResolution(12);
    _lastVoltage = readVoltage();
}

float PowerManager::readVoltage() {
    int raw = analogRead(_vbatPin);
    _lastVoltage = (raw / ADC_RESOLUTION) * ADC_REF_VOLTAGE * VDIV_RATIO;
    return _lastVoltage;
}

uint8_t PowerManager::getPercentage() {
    float v = readVoltage();
    if (v >= VBAT_FULL) return 100;
    if (v <= VBAT_CRITICAL) return 0;
    // Linear approximation between critical and full
    return (uint8_t)(((v - VBAT_CRITICAL) / (VBAT_FULL - VBAT_CRITICAL)) * 100.0f);
}

PowerState PowerManager::getState() {
    float v = readVoltage();
    if (v >= VBAT_NOMINAL) return POWER_FULL;
    if (v >= VBAT_LOW) return POWER_NOMINAL;
    if (v >= VBAT_CRITICAL) return POWER_LOW;
    return POWER_CRITICAL;
}

void PowerManager::deepSleep(uint64_t durationUs) {
    esp_deep_sleep(durationUs);
}

bool PowerManager::shouldThrottle() {
    return getState() >= POWER_LOW;
}
