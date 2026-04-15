#ifndef HEAT_SENSORS_H
#define HEAT_SENSORS_H

#include "../../base/sensor_iface.h"

// Urban Heat Island sensor package.
// Uses DFRobot Multifunctional Environmental Sensor (SEN0500/SEN0501)
// which provides: temperature, humidity, UV intensity, luminous intensity,
// atmospheric pressure, and altitude.
//
// ML model: time-series classification of heat stress conditions
// from temperature + humidity + UV patterns.

// Heat stress levels (based on NOAA Heat Index categories)
enum HeatStressLevel {
    HEAT_NORMAL,     // < 80F / 27C heat index
    HEAT_CAUTION,    // 80-90F / 27-32C
    HEAT_EXTREME_CAUTION, // 90-103F / 32-39C
    HEAT_DANGER,     // 103-124F / 39-51C
    HEAT_EXTREME     // > 124F / 51C
};

class HeatSensorPackage : public SensorPackage {
public:
    bool init() override;
    bool read() override;
    uint8_t getFields(SensorField fields[], uint8_t maxFields) override;
    uint16_t toLoRaPayload(char* buffer, uint16_t bufferSize, uint8_t nodeId) override;
    bool runInference(SensorEvent* event) override;
    const char* getPackageId() override { return "heat"; }
    char getLoRaType() override { return 'H'; }

    // Heat-specific methods
    float getHeatIndex();
    HeatStressLevel getStressLevel();

private:
    // Current readings
    float _temperature = 0.0f;    // Celsius
    float _humidity = 0.0f;       // %RH
    float _uvIntensity = 0.0f;    // mW/cm2
    float _lightIntensity = 0.0f; // Lux
    float _pressure = 0.0f;      // hPa
    float _altitude = 0.0f;      // meters

    // Derived
    float _heatIndex = 0.0f;     // Celsius

    // Time-series buffer for ML inference
    static const uint8_t HISTORY_SIZE = 30; // 30 readings = ~2.5 min at 5s interval
    float _tempHistory[30];
    float _humHistory[30];
    float _uvHistory[30];
    uint8_t _historyIndex = 0;
    bool _historyFull = false;

    // Calculate heat index from temperature and humidity (Rothfusz regression)
    float calculateHeatIndex(float tempC, float humidity);
};

#endif // HEAT_SENSORS_H
