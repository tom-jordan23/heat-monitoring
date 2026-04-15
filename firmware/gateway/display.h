#ifndef GATEWAY_DISPLAY_H
#define GATEWAY_DISPLAY_H

#include <Arduino.h>

// Forward declaration — full NodeInfo defined in gateway_main.ino
struct NodeInfo;

// Gateway TFT display manager.
// Renders a card-based UI showing node status and sensor readings.
// Uses DFRobot 3.5" ILI9488 TFT (480x320).

class GatewayDisplay {
public:
    // Initialize TFT hardware
    void init();

    // Show a simple centered message (used during boot/init)
    void showMessage(const char* msg);

    // Render the full dashboard with node cards
    void render(NodeInfo* nodes, uint8_t nodeCount);

    // Clear the screen
    void clear();

private:
    // Draw a single node card at the given grid position
    void drawNodeCard(NodeInfo* node, uint8_t gridX, uint8_t gridY);

    // Draw status bar at top of screen
    void drawStatusBar(uint8_t nodeCount, uint8_t onlineCount);

    // Color helpers based on sensor type
    uint16_t getTypeColor(char sensorType);
    const char* getTypeName(char sensorType);

    bool _initialized = false;
};

#endif // GATEWAY_DISPLAY_H
