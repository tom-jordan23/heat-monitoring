#include "display.h"

// TODO: Include DFRobot TFT library headers when hardware is available
// #include <DFRobot_GDL.h>

// Screen dimensions (ILI9488)
#define SCREEN_W 480
#define SCREEN_H 320

// Card layout: 2 columns, 3 rows
#define CARD_W   230
#define CARD_H   90
#define CARD_PAD 5
#define CARD_COLS 2
#define CARD_ROWS 3
#define STATUS_BAR_H 30

void GatewayDisplay::init() {
    // TODO: Initialize TFT with actual DFRobot library
    // _tft.begin();
    // _tft.setRotation(1); // Landscape
    _initialized = true;
}

void GatewayDisplay::showMessage(const char* msg) {
    if (!_initialized) return;
    // TODO: Draw centered text on TFT
    Serial.printf("[Display] %s\n", msg);
}

void GatewayDisplay::render(NodeInfo* nodes, uint8_t nodeCount) {
    if (!_initialized) return;

    // Count online nodes
    uint8_t onlineCount = 0;
    for (uint8_t i = 0; i < nodeCount; i++) {
        if (nodes[i].online) onlineCount++;
    }

    drawStatusBar(nodeCount, onlineCount);

    // Draw node cards in grid
    for (uint8_t i = 0; i < nodeCount && i < (CARD_COLS * CARD_ROWS); i++) {
        uint8_t col = i % CARD_COLS;
        uint8_t row = i / CARD_COLS;
        drawNodeCard(&nodes[i], col, row);
    }
}

void GatewayDisplay::clear() {
    if (!_initialized) return;
    // TODO: _tft.fillScreen(0x0000);
}

void GatewayDisplay::drawNodeCard(NodeInfo* node, uint8_t gridX, uint8_t gridY) {
    // Calculate pixel position
    uint16_t x = CARD_PAD + gridX * (CARD_W + CARD_PAD);
    uint16_t y = STATUS_BAR_H + CARD_PAD + gridY * (CARD_H + CARD_PAD);

    // TODO: Render with actual TFT library. For now, serial output.
    Serial.printf("[Display] Card Node %d at (%d,%d): %s %s",
                  node->id, x, y,
                  node->online ? "ONLINE" : "OFFLINE",
                  getTypeName(node->sensorType));

    if (node->eventActive) {
        Serial.printf(" [ALERT: %s]", node->eventDescription);
    }

    // Print sensor values
    for (uint8_t i = 0; i < node->fieldCount; i++) {
        Serial.printf(" f%d=%.1f", i, node->fields[i]);
    }
    Serial.println();
}

void GatewayDisplay::drawStatusBar(uint8_t nodeCount, uint8_t onlineCount) {
    // TODO: Render with TFT
    Serial.printf("[Display] Nodes: %d/%d online\n", onlineCount, nodeCount);
}

uint16_t GatewayDisplay::getTypeColor(char sensorType) {
    switch (sensorType) {
        case 'H': return 0xF800; // Red for heat
        case 'A': return 0x07E0; // Green for air
        case 'F': return 0x001F; // Blue for flood
        case 'W': return 0x07FF; // Cyan for water
        default:  return 0xFFFF; // White
    }
}

const char* GatewayDisplay::getTypeName(char sensorType) {
    switch (sensorType) {
        case 'H': return "HEAT";
        case 'A': return "AIR";
        case 'F': return "FLOOD";
        case 'W': return "WATER";
        default:  return "UNKNOWN";
    }
}
