#ifndef ML_RUNNER_H
#define ML_RUNNER_H

#include <Arduino.h>
#include "sensor_iface.h"

// Generic Edge Impulse inference runner.
// Wraps the EI library's classify() call and manages the
// smoothing / confidence thresholding that Forest-Guard uses.

// Default confidence threshold for triggering an event
#define DEFAULT_CONFIDENCE_THRESHOLD 0.90f

// Exponential moving average smoothing factor (0.0 = no smoothing, 1.0 = no update)
#define DEFAULT_SMOOTHING_ALPHA 0.60f

class MlRunner {
public:
    // Configure thresholds
    void init(float confidenceThreshold = DEFAULT_CONFIDENCE_THRESHOLD,
              float smoothingAlpha = DEFAULT_SMOOTHING_ALPHA);

    // Run inference using the sensor package's runInference method.
    // Applies smoothing and thresholding.
    // Returns true if a confirmed event is detected.
    bool run(SensorPackage* sensorPkg, SensorEvent* event);

    // Get the smoothed confidence value for the last inference
    float getSmoothedConfidence();

    // Reset smoothing state (e.g., after event is cleared)
    void reset();

    // Update confidence threshold at runtime
    void setThreshold(float threshold);

private:
    float _threshold = DEFAULT_CONFIDENCE_THRESHOLD;
    float _alpha = DEFAULT_SMOOTHING_ALPHA;
    float _smoothedConfidence = 0.0f;
    bool _initialized = false;
};

#endif // ML_RUNNER_H
