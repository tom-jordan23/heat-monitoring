#include "heat_sensors.h"

// TODO: Include DFRobot sensor library when hardware is available
// #include <DFRobot_EnvironmentalSensor.h>
// DFRobot_EnvironmentalSensor envSensor(/*addr=*/SEN050X_DEFAULT_DEVICE_ADDRESS, &Wire);

// TODO: Include Edge Impulse model header when trained
// #include "heat_model/edge-impulse-sdk/classifier/ei_run_classifier.h"

bool HeatSensorPackage::init() {
    // TODO: Initialize I2C and DFRobot sensor
    // Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    // if (envSensor.begin() != 0) return false;

    memset(_tempHistory, 0, sizeof(_tempHistory));
    memset(_humHistory, 0, sizeof(_humHistory));
    memset(_uvHistory, 0, sizeof(_uvHistory));
    _historyIndex = 0;
    _historyFull = false;

    Serial.println("[Heat] Sensor package initialized");
    return true;
}

bool HeatSensorPackage::read() {
    // TODO: Read from actual DFRobot sensor
    // _temperature = envSensor.getTemperature(TEMP_C);
    // _humidity = envSensor.getHumidity();
    // _uvIntensity = envSensor.getUltravioletIntensity();
    // _lightIntensity = envSensor.getLuminousIntensity();
    // _pressure = envSensor.getAtmospherePressure(HPA);
    // _altitude = envSensor.getElevation();

    // Placeholder: simulate readings for development
    _temperature = 25.0f + random(-50, 50) / 10.0f;
    _humidity = 60.0f + random(-100, 100) / 10.0f;
    _uvIntensity = 3.0f + random(0, 50) / 10.0f;
    _lightIntensity = 500.0f + random(-200, 200);
    _pressure = 1013.25f + random(-50, 50) / 10.0f;
    _altitude = 100.0f;

    // Calculate derived values
    _heatIndex = calculateHeatIndex(_temperature, _humidity);

    // Store in history buffer for ML
    _tempHistory[_historyIndex] = _temperature;
    _humHistory[_historyIndex] = _humidity;
    _uvHistory[_historyIndex] = _uvIntensity;
    _historyIndex++;
    if (_historyIndex >= HISTORY_SIZE) {
        _historyIndex = 0;
        _historyFull = true;
    }

    return true;
}

uint8_t HeatSensorPackage::getFields(SensorField fields[], uint8_t maxFields) {
    uint8_t count = 0;

    if (count < maxFields) {
        strncpy(fields[count].label, "Temp", sizeof(fields[count].label));
        strncpy(fields[count].unit, "C", sizeof(fields[count].unit));
        fields[count].value = _temperature;
        fields[count].alert = (_temperature > 35.0f);
        count++;
    }
    if (count < maxFields) {
        strncpy(fields[count].label, "Humidity", sizeof(fields[count].label));
        strncpy(fields[count].unit, "%", sizeof(fields[count].unit));
        fields[count].value = _humidity;
        fields[count].alert = false;
        count++;
    }
    if (count < maxFields) {
        strncpy(fields[count].label, "Heat Idx", sizeof(fields[count].label));
        strncpy(fields[count].unit, "C", sizeof(fields[count].unit));
        fields[count].value = _heatIndex;
        fields[count].alert = (getStressLevel() >= HEAT_DANGER);
        count++;
    }
    if (count < maxFields) {
        strncpy(fields[count].label, "UV", sizeof(fields[count].label));
        strncpy(fields[count].unit, "mW/cm2", sizeof(fields[count].unit));
        fields[count].value = _uvIntensity;
        fields[count].alert = (_uvIntensity > 6.0f);
        count++;
    }
    if (count < maxFields) {
        strncpy(fields[count].label, "Light", sizeof(fields[count].label));
        strncpy(fields[count].unit, "lux", sizeof(fields[count].unit));
        fields[count].value = _lightIntensity;
        fields[count].alert = false;
        count++;
    }
    if (count < maxFields) {
        strncpy(fields[count].label, "Pressure", sizeof(fields[count].label));
        strncpy(fields[count].unit, "hPa", sizeof(fields[count].unit));
        fields[count].value = _pressure;
        fields[count].alert = false;
        count++;
    }

    return count;
}

uint16_t HeatSensorPackage::toLoRaPayload(char* buffer, uint16_t bufferSize, uint8_t nodeId) {
    // Format: H,<nodeId>,<temp>,<humidity>,<heatIndex>,<uv>,<light>,<pressure>
    int len = snprintf(buffer, bufferSize,
        "H,%u,%.1f,%.1f,%.1f,%.1f,%.0f,%.1f",
        nodeId, _temperature, _humidity, _heatIndex,
        _uvIntensity, _lightIntensity, _pressure);
    return (len > 0 && len < (int)bufferSize) ? len : 0;
}

bool HeatSensorPackage::runInference(SensorEvent* event) {
    // Need enough history for meaningful inference
    if (!_historyFull) return false;

    // TODO: Replace with actual Edge Impulse inference
    // For now, use threshold-based detection as a placeholder
    // that demonstrates the interface contract.

    HeatStressLevel level = getStressLevel();

    if (level >= HEAT_DANGER) {
        strncpy(event->type, "H+", sizeof(event->type));
        event->severity = (level == HEAT_EXTREME) ? SEVERITY_CRITICAL : SEVERITY_WARNING;

        if (level == HEAT_EXTREME) {
            strncpy(event->description, "EXTREME heat stress detected", sizeof(event->description));
            event->confidence = 0.98f;
        } else {
            strncpy(event->description, "Dangerous heat stress conditions", sizeof(event->description));
            event->confidence = 0.92f;
        }
        return true;
    }

    return false;
}

float HeatSensorPackage::getHeatIndex() {
    return _heatIndex;
}

HeatStressLevel HeatSensorPackage::getStressLevel() {
    if (_heatIndex < 27.0f) return HEAT_NORMAL;
    if (_heatIndex < 32.0f) return HEAT_CAUTION;
    if (_heatIndex < 39.0f) return HEAT_EXTREME_CAUTION;
    if (_heatIndex < 51.0f) return HEAT_DANGER;
    return HEAT_EXTREME;
}

float HeatSensorPackage::calculateHeatIndex(float tempC, float humidity) {
    // Convert to Fahrenheit for the Rothfusz regression equation
    float T = tempC * 9.0f / 5.0f + 32.0f;
    float R = humidity;

    // Simple formula for low values
    float HI = 0.5f * (T + 61.0f + ((T - 68.0f) * 1.2f) + (R * 0.094f));

    if (HI >= 80.0f) {
        // Full Rothfusz regression
        HI = -42.379f
             + 2.04901523f * T
             + 10.14333127f * R
             - 0.22475541f * T * R
             - 0.00683783f * T * T
             - 0.05481717f * R * R
             + 0.00122874f * T * T * R
             + 0.00085282f * T * R * R
             - 0.00000199f * T * T * R * R;

        // Adjustments
        if (R < 13.0f && T >= 80.0f && T <= 112.0f) {
            HI -= ((13.0f - R) / 4.0f) * sqrt((17.0f - abs(T - 95.0f)) / 17.0f);
        } else if (R > 85.0f && T >= 80.0f && T <= 87.0f) {
            HI += ((R - 85.0f) / 10.0f) * ((87.0f - T) / 5.0f);
        }
    }

    // Convert back to Celsius
    return (HI - 32.0f) * 5.0f / 9.0f;
}
