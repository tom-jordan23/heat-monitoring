#include "firebase.h"

// TODO: Include HTTP client library when building for Arduino Uno R4 WiFi
// #include <HttpClient.h>
// #include <WiFiS3.h>

void FirebaseClient::init(const char* projectUrl, const char* authToken) {
    _projectUrl = projectUrl;
    _authToken = authToken;
    memset(_hasBaseline, 0, sizeof(_hasBaseline));
}

void FirebaseClient::writeTelemetry(uint8_t nodeId, char sensorType,
                                     float* fields, uint8_t fieldCount) {
    if (!_projectUrl || !_authToken) return;
    if (nodeId >= 16 || fieldCount > 8) return;

    // Change detection — skip write if values haven't changed enough
    if (!hasChanged(nodeId, fields, fieldCount)) return;

    // Build JSON payload
    char json[256];
    int offset = snprintf(json, sizeof(json),
        "{\"type\":\"%c\",\"node\":%d,\"fields\":[", sensorType, nodeId);

    for (uint8_t i = 0; i < fieldCount; i++) {
        offset += snprintf(json + offset, sizeof(json) - offset,
            "%.2f%s", fields[i], (i < fieldCount - 1) ? "," : "");
    }
    snprintf(json + offset, sizeof(json) - offset, "]}");

    // Write to Firebase
    char path[64];
    snprintf(path, sizeof(path), "/nodes/%d/telemetry.json", nodeId);
    httpPut(path, json);

    // Update baseline
    memcpy(_lastFields[nodeId], fields, fieldCount * sizeof(float));
    _hasBaseline[nodeId] = true;
}

void FirebaseClient::writeEvent(uint8_t nodeId, char sensorType,
                                 uint16_t eventId, float confidence) {
    if (!_projectUrl || !_authToken) return;

    char json[128];
    snprintf(json, sizeof(json),
        "{\"type\":\"%c\",\"node\":%d,\"event\":%d,\"confidence\":%.2f,\"active\":true}",
        sensorType, nodeId, eventId, confidence);

    char path[64];
    snprintf(path, sizeof(path), "/nodes/%d/event.json", nodeId);
    httpPut(path, json);
}

bool FirebaseClient::shouldClearEvent() {
    // TODO: HTTP GET /meta/Event.json and check if false
    return false;
}

bool FirebaseClient::httpPut(const char* path, const char* json) {
    // TODO: Implement with WiFiS3 HttpClient
    // For now, log to serial
    Serial.printf("[Firebase] PUT %s: %s\n", path, json);
    return true;
}

bool FirebaseClient::hasChanged(uint8_t nodeId, float* fields, uint8_t fieldCount) {
    if (!_hasBaseline[nodeId]) return true;

    for (uint8_t i = 0; i < fieldCount; i++) {
        float delta = abs(fields[i] - _lastFields[nodeId][i]);
        if (delta >= TELEMETRY_DELTA_THRESHOLD) return true;
    }
    return false;
}
