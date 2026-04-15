#ifndef LORA_COMM_H
#define LORA_COMM_H

#include <Arduino.h>

// LoRa message framing: #<body>*
// Body format varies by message type:
//   Registration:  <nodeId>
//   Telemetry:     <type>,<nodeId>,<field1>,<field2>,...
//   Event:         <type>+<eventId>,<nodeId>,<data...>,<date>,<time>
//   Gateway ACK:   +OK
//   Gateway Clear: +C

#define LORA_FRAME_START '#'
#define LORA_FRAME_END   '*'
#define LORA_BAUD_RATE   115200
#define LORA_MAX_MSG     256

// Parsed LoRa message
struct LoRaMessage {
    char body[LORA_MAX_MSG];    // Raw body between # and *
    uint16_t length;            // Length of body
    bool valid;                 // True if frame was valid
};

class LoRaComm {
public:
    // Initialize LoRa UART on the given serial port
    void init(HardwareSerial& serial);

    // Send a framed message: #<body>*
    void send(const char* body);

    // Send a registration message for this node
    void sendRegistration(uint8_t nodeId);

    // Send a telemetry payload (already formatted by sensor package)
    void sendTelemetry(const char* payload);

    // Send an event message
    // type: event type code (e.g. "H+", "A+", "F+", "W+")
    // eventId: unique event identifier
    // nodeId: originating node
    // data: event-specific data string
    // date: date string (YYYY-MM-DD)
    // time: time string (HH:MM:SS)
    void sendEvent(const char* type, uint16_t eventId, uint8_t nodeId,
                   const char* data, const char* date, const char* time);

    // Try to receive a message. Non-blocking.
    // Returns true if a complete message was received.
    bool receive(LoRaMessage* msg);

    // Check if received message is a gateway ACK
    static bool isAck(const LoRaMessage& msg);

    // Check if received message is a clear command
    static bool isClear(const LoRaMessage& msg);

private:
    HardwareSerial* _serial = nullptr;
    char _rxBuffer[LORA_MAX_MSG];
    uint16_t _rxIndex = 0;
    bool _rxInFrame = false;
};

#endif // LORA_COMM_H
