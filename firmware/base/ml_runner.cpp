#include "ml_runner.h"

void MlRunner::init(float confidenceThreshold, float smoothingAlpha) {
    _threshold = confidenceThreshold;
    _alpha = smoothingAlpha;
    _smoothedConfidence = 0.0f;
    _initialized = true;
}

bool MlRunner::run(SensorPackage* sensorPkg, SensorEvent* event) {
    if (!_initialized || !sensorPkg) return false;

    // Run the sensor package's inference method
    bool detected = sensorPkg->runInference(event);

    if (detected) {
        // Apply exponential moving average smoothing to confidence
        _smoothedConfidence = (_alpha * _smoothedConfidence) +
                              ((1.0f - _alpha) * event->confidence);

        // Only confirm event if smoothed confidence exceeds threshold
        if (_smoothedConfidence >= _threshold) {
            event->confidence = _smoothedConfidence;
            return true;
        }
    } else {
        // Decay smoothed confidence when no detection
        _smoothedConfidence *= _alpha;
    }

    return false;
}

float MlRunner::getSmoothedConfidence() {
    return _smoothedConfidence;
}

void MlRunner::reset() {
    _smoothedConfidence = 0.0f;
}

void MlRunner::setThreshold(float threshold) {
    _threshold = threshold;
}
