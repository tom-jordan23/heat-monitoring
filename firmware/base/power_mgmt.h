#ifndef POWER_MGMT_H
#define POWER_MGMT_H

#include <Arduino.h>

// Battery and solar power management for sensor nodes.
// Based on TP4056 charging controller + XC6210B332MR 3.3V regulator.

// Default ADC pin for battery voltage divider (adjust per PCB revision)
#define DEFAULT_VBAT_PIN 4

// Voltage thresholds for LiPo (3.7V nominal)
#define VBAT_FULL     4.15f   // Fully charged
#define VBAT_NOMINAL  3.70f   // Nominal
#define VBAT_LOW      3.40f   // Low warning
#define VBAT_CRITICAL 3.20f   // Critical — should sleep

enum PowerState {
    POWER_FULL,
    POWER_NOMINAL,
    POWER_LOW,
    POWER_CRITICAL
};

class PowerManager {
public:
    // Initialize ADC for battery monitoring
    void init(uint8_t vbatPin = DEFAULT_VBAT_PIN);

    // Read current battery voltage
    float readVoltage();

    // Get battery percentage (0-100)
    uint8_t getPercentage();

    // Get current power state
    PowerState getState();

    // Enter deep sleep for the given duration (microseconds).
    // Use when battery is critical to preserve charge until solar recharges.
    void deepSleep(uint64_t durationUs);

    // Check if we should reduce activity to save power
    bool shouldThrottle();

private:
    uint8_t _vbatPin = DEFAULT_VBAT_PIN;
    float _lastVoltage = 0.0f;
};

#endif // POWER_MGMT_H
