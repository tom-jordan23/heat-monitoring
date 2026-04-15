#ifndef SENSOR_IFACE_H
#define SENSOR_IFACE_H

#include <Arduino.h>

// Maximum number of telemetry fields a sensor package can report
#define MAX_SENSOR_FIELDS 8

// Maximum LoRa payload size (excluding framing chars # and *)
#define MAX_LORA_PAYLOAD 200

// A single named sensor reading for display and telemetry
struct SensorField {
    char label[16];     // Short label, e.g. "Temp", "PM2.5"
    char unit[8];       // Unit string, e.g. "C", "ug/m3"
    float value;        // Current reading
    bool alert;         // True if this field is in an alert state
};

// Event severity levels
enum EventSeverity {
    SEVERITY_NONE = 0,
    SEVERITY_INFO = 1,
    SEVERITY_WARNING = 2,
    SEVERITY_CRITICAL = 3
};

// An event detected by ML inference or threshold logic
struct SensorEvent {
    char type[4];              // LoRa message type code, e.g. "H+" (heat), "A+" (air)
    char description[64];      // Human-readable event description
    EventSeverity severity;
    float confidence;          // ML confidence score (0.0 - 1.0)
};

// Abstract interface that all sensor packages implement.
// The node firmware calls these methods without knowing which
// sensors are physically attached — that's determined at compile time.
class SensorPackage {
public:
    virtual ~SensorPackage() {}

    // Initialize sensor hardware. Called once during setup().
    // Returns true if all sensors initialized successfully.
    virtual bool init() = 0;

    // Read all sensors. Called periodically from Core 0.
    // Returns true if readings are valid.
    virtual bool read() = 0;

    // Get the current sensor readings for display and telemetry.
    // Returns the number of fields populated (up to MAX_SENSOR_FIELDS).
    virtual uint8_t getFields(SensorField fields[], uint8_t maxFields) = 0;

    // Serialize current readings into a LoRa telemetry payload.
    // Format: "E,<nodeId>,<field1>,<field2>,..."
    // Returns the number of bytes written to buffer.
    virtual uint16_t toLoRaPayload(char* buffer, uint16_t bufferSize, uint8_t nodeId) = 0;

    // Feed current readings to the Edge Impulse model for inference.
    // Called from Core 1. Returns true if an event was detected.
    // If true, the event struct is populated with details.
    virtual bool runInference(SensorEvent* event) = 0;

    // Get the package identifier string, e.g. "heat", "air", "flood", "water"
    virtual const char* getPackageId() = 0;

    // Get the LoRa telemetry message type character for this package
    // Standard types: 'H' (heat), 'A' (air), 'F' (flood), 'W' (water)
    virtual char getLoRaType() = 0;

    // Optional: return the recommended read interval in milliseconds.
    // Default is 5000ms. Override for sensors that need faster/slower polling.
    virtual uint32_t getReadIntervalMs() { return 5000; }

    // Optional: return the recommended inference interval in milliseconds.
    // Default is 1000ms.
    virtual uint32_t getInferenceIntervalMs() { return 1000; }
};

#endif // SENSOR_IFACE_H
