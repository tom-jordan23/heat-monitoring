#ifndef FIREBASE_CLIENT_H
#define FIREBASE_CLIENT_H

#include <Arduino.h>

// Firebase Realtime Database REST client for the gateway.
// Writes telemetry and event data via HTTP PUT/PATCH.

// Change detection thresholds to reduce Firebase writes
#define TELEMETRY_DELTA_THRESHOLD 1.0f   // Only write on >=1.0 unit change
#define LOCATION_DELTA_THRESHOLD  0.00010f // ~11 meter displacement

class FirebaseClient {
public:
    // Initialize with Firebase project URL and auth token.
    // These should be set before first use — defaults are placeholders.
    void init(const char* projectUrl = "YOUR_PROJECT.firebaseio.com",
              const char* authToken = "YOUR_AUTH_TOKEN");

    // Write telemetry data for a node
    void writeTelemetry(uint8_t nodeId, char sensorType,
                        float* fields, uint8_t fieldCount);

    // Write an event
    void writeEvent(uint8_t nodeId, char sensorType,
                    uint16_t eventId, float confidence);

    // Check if the gateway should clear an event (reads meta/Event from Firebase)
    bool shouldClearEvent();

private:
    // Send an HTTP PUT request to Firebase
    bool httpPut(const char* path, const char* json);

    // Check if telemetry has changed enough to warrant a write
    bool hasChanged(uint8_t nodeId, float* fields, uint8_t fieldCount);

    const char* _projectUrl = nullptr;
    const char* _authToken = nullptr;

    // Last written values for change detection (per node)
    float _lastFields[16][8];  // Up to 16 nodes, 8 fields each
    bool _hasBaseline[16];
};

#endif // FIREBASE_CLIENT_H
