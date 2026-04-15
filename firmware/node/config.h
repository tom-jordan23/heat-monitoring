#ifndef NODE_CONFIG_H
#define NODE_CONFIG_H

// ============================================================
// Node Configuration
// Edit this file to configure each node before flashing.
// ============================================================

// --- Node Identity ---
#define NODE_ID 1   // Unique ID for this node (1-255)

// --- Sensor Package Selection ---
// Uncomment exactly ONE of these to select which sensor package to compile.
#define SENSOR_PACKAGE_HEAT
// #define SENSOR_PACKAGE_AIR
// #define SENSOR_PACKAGE_FLOOD
// #define SENSOR_PACKAGE_WATER

// --- Pin Assignments (ESP32-S3) ---
#define PIN_LORA_RX     16    // UART RX from LoRa module
#define PIN_LORA_TX     17    // UART TX to LoRa module
#define PIN_NEOPIXEL    48    // Status LED
#define PIN_VBAT        4     // Battery voltage ADC

// --- I2C (for DFRobot sensors) ---
#define PIN_I2C_SDA     8
#define PIN_I2C_SCL     9

// --- Timing ---
#define REGISTRATION_INTERVAL_MS   10000  // Re-register every 10s until ACK
#define TELEMETRY_INTERVAL_MS      30000  // Send telemetry every 30s
#define EVENT_REPEAT_INTERVAL_MS   5000   // Re-send event every 5s until cleared

// --- ML Inference ---
#define ML_CONFIDENCE_THRESHOLD    0.90f
#define ML_SMOOTHING_ALPHA         0.60f

// --- Power Management ---
#define LOW_POWER_TELEMETRY_INTERVAL_MS  60000  // Slower telemetry when battery low
#define CRITICAL_SLEEP_DURATION_US       300000000ULL  // 5 min deep sleep when critical

#endif // NODE_CONFIG_H
