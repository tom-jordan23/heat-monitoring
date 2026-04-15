# LoRa Protocol Specification

## Overview

All messages between sensor nodes and the gateway use a simple framed text protocol over UART-connected LoRa modules running Meshtastic firmware.

## Frame Format

```
#<body>*
```

- `#` — frame start delimiter
- `*` — frame end delimiter
- `<body>` — variable-length ASCII payload (max 200 bytes)

## Message Types

### Registration (Node → Gateway)

Sent periodically until acknowledged. Body is just the node ID.

```
#<nodeId>*
```

**Example:** `#01*` — Node 1 requesting registration

### Gateway Acknowledgment (Gateway → Node)

```
#+OK*
```

### Gateway Clear Event (Gateway → Broadcast)

Clears the active event on all nodes.

```
#+C*
```

### Telemetry (Node → Gateway)

Periodic sensor readings. First field is the sensor type character, second is the node ID, remaining fields are sensor-specific.

```
#<type>,<nodeId>,<field1>,<field2>,...*
```

**Type codes:**

| Code | Package | Fields |
|------|---------|--------|
| `H` | Heat | temp, humidity, heatIndex, uv, light, pressure |
| `A` | Air | pm25, pm10, voc, co, ozone, aqi |
| `F` | Flood | waterLevel, rainRate, soilMoisture, pressure |
| `W` | Water | turbidity, pH, dissolvedO2, conductivity, temp |

**Examples:**
```
#H,1,28.5,65.2,31.4,3.2,850,1013.2*    — Heat telemetry from Node 1
#A,3,12.5,25.0,0.8,0.2,0.04,52*         — Air quality from Node 3
#F,2,1.23,5.6,78.3,1010.5*              — Flood sensors from Node 2
#W,4,15.2,7.1,8.5,450.0,22.3*           — Water quality from Node 4
```

### Event (Node → Gateway)

Triggered when ML inference detects a significant condition.

```
#<type>+<eventId>,<nodeId>,<data>,<date>,<time>*
```

- `<type>+` — sensor type code followed by `+` (e.g., `H+`, `A+`, `F+`, `W+`)
- `<eventId>` — incrementing event counter per node
- `<data>` — event-specific data (typically ML confidence score)
- `<date>` — `YYYY-MM-DD`
- `<time>` — `HH:MM:SS`

**Examples:**
```
#H+1,1,0.95,2026-07-15,14:30:22*   — Heat stress event from Node 1
#F+3,2,0.91,2026-03-20,03:15:44*   — Flood risk event from Node 2
```

## Adding New Sensor Packages

To add a new sensor type:

1. Choose a single uppercase letter as the type code (must be unique)
2. Define the telemetry field order in this document
3. Implement `toLoRaPayload()` in your sensor package to emit the correct format
4. Update the gateway's `handleTelemetry()` to parse the new fields
5. Add display cards for the new type

## Constraints

- Maximum body length: 200 bytes (defined by `MAX_LORA_PAYLOAD`)
- All numeric values are ASCII-encoded decimal
- Field separator: `,`
- No spaces in messages
- Node IDs: 1–255 (0 reserved)
- Event IDs: 16-bit unsigned (0–65535), wraps around

## Physical Layer

- **Module:** Waveshare RP2040 LoRa (SX1262)
- **Firmware:** Meshtastic
- **Interface:** UART at 115200 baud
- **Topology:** Peer-to-peer mesh with gateway as sink
- **Range:** ~1-10km depending on terrain and antenna
