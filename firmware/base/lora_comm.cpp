#include "lora_comm.h"

void LoRaComm::init(HardwareSerial& serial) {
    _serial = &serial;
    _serial->begin(LORA_BAUD_RATE);
    _rxIndex = 0;
    _rxInFrame = false;
}

void LoRaComm::send(const char* body) {
    if (!_serial) return;
    _serial->write(LORA_FRAME_START);
    _serial->print(body);
    _serial->write(LORA_FRAME_END);
}

void LoRaComm::sendRegistration(uint8_t nodeId) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%02u", nodeId);
    send(buf);
}

void LoRaComm::sendTelemetry(const char* payload) {
    send(payload);
}

void LoRaComm::sendEvent(const char* type, uint16_t eventId, uint8_t nodeId,
                          const char* data, const char* date, const char* time) {
    char buf[LORA_MAX_MSG];
    snprintf(buf, sizeof(buf), "%s%u,%u,%s,%s,%s",
             type, eventId, nodeId, data, date, time);
    send(buf);
}

bool LoRaComm::receive(LoRaMessage* msg) {
    if (!_serial) return false;

    while (_serial->available()) {
        char c = _serial->read();

        if (c == LORA_FRAME_START) {
            _rxIndex = 0;
            _rxInFrame = true;
            continue;
        }

        if (c == LORA_FRAME_END && _rxInFrame) {
            _rxBuffer[_rxIndex] = '\0';
            msg->length = _rxIndex;
            memcpy(msg->body, _rxBuffer, _rxIndex + 1);
            msg->valid = true;
            _rxInFrame = false;
            _rxIndex = 0;
            return true;
        }

        if (_rxInFrame && _rxIndex < LORA_MAX_MSG - 1) {
            _rxBuffer[_rxIndex++] = c;
        }
    }

    return false;
}

bool LoRaComm::isAck(const LoRaMessage& msg) {
    return msg.valid && strcmp(msg.body, "+OK") == 0;
}

bool LoRaComm::isClear(const LoRaMessage& msg) {
    return msg.valid && strcmp(msg.body, "+C") == 0;
}
