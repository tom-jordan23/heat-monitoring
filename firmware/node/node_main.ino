// Community Sentinel — Node Firmware
// ESP32-S3 dual-core: Core 0 handles sensors + LoRa, Core 1 handles ML inference.
// Sensor package is selected at compile time via config.h.

#include "config.h"
#include "../base/sensor_iface.h"
#include "../base/lora_comm.h"
#include "../base/power_mgmt.h"
#include "../base/led_status.h"
#include "../base/ml_runner.h"

// --- Include the selected sensor package ---
#if defined(SENSOR_PACKAGE_HEAT)
  #include "../sensor_packages/heat/heat_sensors.h"
  HeatSensorPackage sensorPkg;
#elif defined(SENSOR_PACKAGE_AIR)
  #include "../sensor_packages/air/air_sensors.h"
  AirSensorPackage sensorPkg;
#elif defined(SENSOR_PACKAGE_FLOOD)
  #include "../sensor_packages/flood/flood_sensors.h"
  FloodSensorPackage sensorPkg;
#elif defined(SENSOR_PACKAGE_WATER)
  #include "../sensor_packages/water/water_sensors.h"
  WaterSensorPackage sensorPkg;
#else
  #error "No sensor package selected! Define one in config.h"
#endif

// --- Globals ---
LoRaComm lora;
PowerManager power;
StatusLed led;
MlRunner mlRunner;

// Node state machine
enum NodeState {
    STATE_INIT,
    STATE_REGISTERING,
    STATE_RUNNING,
    STATE_EVENT_ACTIVE,
    STATE_LOW_POWER,
    STATE_SLEEPING
};

volatile NodeState nodeState = STATE_INIT;
volatile bool eventActive = false;
SensorEvent currentEvent;
uint16_t eventCounter = 0;

// FreeRTOS mutex for shared state
SemaphoreHandle_t stateMutex;

// Timing
uint32_t lastRegistration = 0;
uint32_t lastTelemetry = 0;
uint32_t lastEventSend = 0;
uint32_t lastSensorRead = 0;

// --- Core 0: Sensors, LoRa, State Management ---
void setup() {
    Serial.begin(115200);
    Serial.printf("[Node %d] Booting...\n", NODE_ID);

    stateMutex = xSemaphoreCreateMutex();

    // Initialize subsystems
    led.init(PIN_NEOPIXEL);
    led.setPattern(LED_SOLID_BLUE);

    power.init(PIN_VBAT);
    Serial.printf("[Node %d] Battery: %.2fV (%d%%)\n",
                  NODE_ID, power.readVoltage(), power.getPercentage());

    // Check for critical battery before proceeding
    if (power.getState() == POWER_CRITICAL) {
        Serial.printf("[Node %d] Battery critical! Sleeping...\n", NODE_ID);
        power.deepSleep(CRITICAL_SLEEP_DURATION_US);
    }

    // Initialize LoRa on Serial1
    Serial1.begin(LORA_BAUD_RATE, SERIAL_8N1, PIN_LORA_RX, PIN_LORA_TX);
    lora.init(Serial1);

    // Initialize sensor package
    if (!sensorPkg.init()) {
        Serial.printf("[Node %d] Sensor init FAILED!\n", NODE_ID);
        led.setPattern(LED_BREATHING_RED);
        while (true) { led.update(); delay(10); }
    }
    Serial.printf("[Node %d] Sensor package '%s' initialized\n",
                  NODE_ID, sensorPkg.getPackageId());

    // Initialize ML runner
    mlRunner.init(ML_CONFIDENCE_THRESHOLD, ML_SMOOTHING_ALPHA);

    // Start ML inference task on Core 1
    xTaskCreatePinnedToCore(
        inferenceTask,    // Function
        "ML_Inference",   // Name
        8192,             // Stack size
        NULL,             // Parameters
        1,                // Priority
        NULL,             // Task handle
        1                 // Core 1
    );

    nodeState = STATE_REGISTERING;
    Serial.printf("[Node %d] Ready. Registering with gateway...\n", NODE_ID);
}

void loop() {
    uint32_t now = millis();

    // Update LED animation
    led.update();

    // Check power state
    if (power.shouldThrottle() && nodeState == STATE_RUNNING) {
        xSemaphoreTake(stateMutex, portMAX_DELAY);
        nodeState = STATE_LOW_POWER;
        xSemaphoreGive(stateMutex);
        led.setPattern(LED_BREATHING_YELLOW);
    }

    // Handle LoRa incoming messages
    LoRaMessage rxMsg;
    if (lora.receive(&rxMsg)) {
        handleLoRaMessage(rxMsg);
    }

    // State machine
    switch (nodeState) {
        case STATE_REGISTERING:
            if (now - lastRegistration >= REGISTRATION_INTERVAL_MS) {
                lora.sendRegistration(NODE_ID);
                led.flash(0x0000FF, 100);
                lastRegistration = now;
            }
            break;

        case STATE_RUNNING:
        case STATE_LOW_POWER: {
            uint32_t telemetryInterval = (nodeState == STATE_LOW_POWER)
                ? LOW_POWER_TELEMETRY_INTERVAL_MS
                : TELEMETRY_INTERVAL_MS;

            // Read sensors
            if (now - lastSensorRead >= sensorPkg.getReadIntervalMs()) {
                sensorPkg.read();
                lastSensorRead = now;
            }

            // Send telemetry
            if (now - lastTelemetry >= telemetryInterval) {
                sendTelemetry();
                lastTelemetry = now;
            }
            break;
        }

        case STATE_EVENT_ACTIVE:
            // Re-send event periodically until gateway clears it
            if (now - lastEventSend >= EVENT_REPEAT_INTERVAL_MS) {
                sendCurrentEvent();
                lastEventSend = now;
            }
            // Still read sensors during events
            if (now - lastSensorRead >= sensorPkg.getReadIntervalMs()) {
                sensorPkg.read();
                lastSensorRead = now;
            }
            break;

        default:
            break;
    }

    delay(10); // Yield to RTOS
}

// --- Core 1: ML Inference Task ---
void inferenceTask(void* params) {
    while (true) {
        if (nodeState == STATE_RUNNING || nodeState == STATE_EVENT_ACTIVE) {
            SensorEvent event;
            bool detected = mlRunner.run(&sensorPkg, &event);

            if (detected && !eventActive) {
                xSemaphoreTake(stateMutex, portMAX_DELAY);
                eventActive = true;
                eventCounter++;
                currentEvent = event;
                nodeState = STATE_EVENT_ACTIVE;
                xSemaphoreGive(stateMutex);

                led.setPattern(LED_BREATHING_RED);
                Serial.printf("[Node %d] EVENT: %s (conf=%.2f)\n",
                              NODE_ID, event.description, event.confidence);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(sensorPkg.getInferenceIntervalMs()));
    }
}

// --- Helper Functions ---

void handleLoRaMessage(const LoRaMessage& msg) {
    if (LoRaComm::isAck(msg)) {
        if (nodeState == STATE_REGISTERING) {
            xSemaphoreTake(stateMutex, portMAX_DELAY);
            nodeState = STATE_RUNNING;
            xSemaphoreGive(stateMutex);
            led.setPattern(LED_SOLID_GREEN);
            Serial.printf("[Node %d] Registered with gateway\n", NODE_ID);
        }
    } else if (LoRaComm::isClear(msg)) {
        if (nodeState == STATE_EVENT_ACTIVE) {
            xSemaphoreTake(stateMutex, portMAX_DELAY);
            eventActive = false;
            nodeState = STATE_RUNNING;
            xSemaphoreGive(stateMutex);
            mlRunner.reset();
            led.setPattern(LED_SOLID_GREEN);
            Serial.printf("[Node %d] Event cleared by gateway\n", NODE_ID);
        }
    }
}

void sendTelemetry() {
    char payload[MAX_LORA_PAYLOAD];
    uint16_t len = sensorPkg.toLoRaPayload(payload, sizeof(payload), NODE_ID);
    if (len > 0) {
        lora.sendTelemetry(payload);
        led.flash(0x00FF00, 50);
    }
}

void sendCurrentEvent() {
    char data[64];
    snprintf(data, sizeof(data), "%.2f", currentEvent.confidence);

    // TODO: get real date/time from RTC or NTP
    lora.sendEvent(currentEvent.type, eventCounter, NODE_ID,
                   data, "2026-01-01", "00:00:00");
    led.flash(0xFF0000, 100);
}
