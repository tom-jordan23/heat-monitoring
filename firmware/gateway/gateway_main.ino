// Community Sentinel — Gateway Firmware
// Arduino Uno R4 WiFi: receives LoRa messages, displays on TFT, pushes to Firebase.

#include <WiFiS3.h>
#include <HttpClient.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include "../base/lora_comm.h"
#include "display.h"
#include "firebase.h"

// --- Configuration ---
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#define MAX_NODES     16
#define DISPLAY_REFRESH_MS 200

// --- Node tracking ---
struct NodeInfo {
    uint8_t id;
    bool online;
    uint32_t lastSeen;
    char sensorType;           // 'H', 'A', 'F', 'W'
    float fields[8];           // Latest telemetry values
    uint8_t fieldCount;
    bool eventActive;
    char eventDescription[64];
    float eventConfidence;
};

NodeInfo nodes[MAX_NODES];
uint8_t nodeCount = 0;

// --- Subsystems ---
LoRaComm lora;
GatewayDisplay display;
FirebaseClient firebase;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

uint32_t lastDisplayRefresh = 0;
bool wifiConnected = false;

void setup() {
    Serial.begin(115200);
    Serial.println("[Gateway] Booting...");

    // Initialize display
    display.init();
    display.showMessage("Initializing...");

    // Initialize LoRa on Serial1
    Serial1.begin(LORA_BAUD_RATE);
    lora.init(Serial1);

    // Connect WiFi
    connectWiFi();

    // Initialize NTP
    if (wifiConnected) {
        timeClient.begin();
        timeClient.update();
    }

    // Initialize Firebase
    firebase.init();

    display.showMessage("Waiting for nodes...");
    Serial.println("[Gateway] Ready. Listening for nodes.");
}

void loop() {
    uint32_t now = millis();

    // Update NTP periodically
    if (wifiConnected) {
        timeClient.update();
    }

    // Receive LoRa messages
    LoRaMessage msg;
    if (lora.receive(&msg)) {
        handleMessage(msg);
    }

    // Refresh display
    if (now - lastDisplayRefresh >= DISPLAY_REFRESH_MS) {
        display.render(nodes, nodeCount);
        lastDisplayRefresh = now;
    }

    // Check for stale nodes (no message in 60s)
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodes[i].online && (now - nodes[i].lastSeen > 60000)) {
            nodes[i].online = false;
            Serial.printf("[Gateway] Node %d went offline\n", nodes[i].id);
        }
    }
}

// --- Message Handling ---

void handleMessage(const LoRaMessage& msg) {
    if (!msg.valid || msg.length == 0) return;

    char body[LORA_MAX_MSG];
    strncpy(body, msg.body, sizeof(body));
    body[sizeof(body) - 1] = '\0';

    // Check message type by looking at the content
    if (isRegistration(body)) {
        handleRegistration(body);
    } else if (isTelemetry(body)) {
        handleTelemetry(body);
    } else if (isEvent(body)) {
        handleEvent(body);
    }
}

bool isRegistration(const char* body) {
    // Registration is just a node ID number (1-2 digits)
    int len = strlen(body);
    if (len > 3) return false;
    for (int i = 0; i < len; i++) {
        if (!isdigit(body[i])) return false;
    }
    return true;
}

bool isTelemetry(const char* body) {
    // Telemetry starts with a type letter followed by comma
    return (strlen(body) > 2 && isalpha(body[0]) && body[1] == ',');
}

bool isEvent(const char* body) {
    // Events have a type letter followed by '+'
    return (strlen(body) > 2 && isalpha(body[0]) && body[1] == '+');
}

void handleRegistration(const char* body) {
    uint8_t nodeId = atoi(body);
    NodeInfo* node = findOrCreateNode(nodeId);
    if (node) {
        node->online = true;
        node->lastSeen = millis();
        lora.send("+OK");
        Serial.printf("[Gateway] Node %d registered\n", nodeId);
    }
}

void handleTelemetry(const char* body) {
    // Format: <type>,<nodeId>,<field1>,<field2>,...
    char type = body[0];
    char* rest = (char*)body + 2; // Skip "X,"

    // Parse node ID
    char* token = strtok(rest, ",");
    if (!token) return;
    uint8_t nodeId = atoi(token);

    NodeInfo* node = findOrCreateNode(nodeId);
    if (!node) return;

    node->sensorType = type;
    node->lastSeen = millis();
    node->online = true;
    node->fieldCount = 0;

    // Parse remaining fields as floats
    while ((token = strtok(NULL, ",")) != NULL && node->fieldCount < 8) {
        node->fields[node->fieldCount++] = atof(token);
    }

    // Push to Firebase (with change detection to reduce writes)
    if (wifiConnected && timeClient.getEpochTime() >= 1735689600) {
        firebase.writeTelemetry(nodeId, type, node->fields, node->fieldCount);
    }
}

void handleEvent(const char* body) {
    // Format: <type>+<eventId>,<nodeId>,<data>,<date>,<time>
    char type = body[0];

    char* rest = (char*)body + 2; // Skip "X+"
    char* token = strtok(rest, ",");
    if (!token) return;
    uint16_t eventId = atoi(token);

    token = strtok(NULL, ",");
    if (!token) return;
    uint8_t nodeId = atoi(token);

    NodeInfo* node = findOrCreateNode(nodeId);
    if (!node) return;

    node->eventActive = true;
    node->lastSeen = millis();

    // Parse confidence from data field
    token = strtok(NULL, ",");
    if (token) {
        node->eventConfidence = atof(token);
    }

    snprintf(node->eventDescription, sizeof(node->eventDescription),
             "Event %c-%u (%.0f%%)", type, eventId, node->eventConfidence * 100);

    Serial.printf("[Gateway] Event from Node %d: %s\n", nodeId, node->eventDescription);

    // Push event to Firebase
    if (wifiConnected) {
        firebase.writeEvent(nodeId, type, eventId, node->eventConfidence);
    }

    // Buzzer alert could go here
}

// --- Node Management ---

NodeInfo* findOrCreateNode(uint8_t nodeId) {
    // Find existing
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodes[i].id == nodeId) return &nodes[i];
    }
    // Create new
    if (nodeCount < MAX_NODES) {
        NodeInfo* node = &nodes[nodeCount++];
        memset(node, 0, sizeof(NodeInfo));
        node->id = nodeId;
        return node;
    }
    return nullptr;
}

// --- WiFi ---

void connectWiFi() {
    Serial.printf("[Gateway] Connecting to %s...\n", WIFI_SSID);
    display.showMessage("Connecting WiFi...");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.printf("[Gateway] WiFi connected: %s\n", WiFi.localIP().toString().c_str());
    } else {
        wifiConnected = false;
        Serial.println("[Gateway] WiFi failed — running in offline mode");
    }
}
