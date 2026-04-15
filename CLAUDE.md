# Environmental Monitoring Platform — Project Guide

## What This Is
A modular edge-AI environmental monitoring platform built on ESP32-S3 + Edge Impulse + LoRa mesh (Meshtastic). Based on the [Forest-Guard](https://github.com/MukeshSankhla/Forest-Guard/) architecture.

The platform has a common base (firmware, LoRa protocol, gateway, display) and pluggable sensor packages for different environmental threats.

## Architecture

```
Display Layer:  Gateway TFT | Web Dashboard | Alerts
Gateway Layer:  Arduino Uno R4 WiFi + LoRa RX + Firebase
Network Layer:  LoRa Mesh (Meshtastic on RP2040)
Node Layer:     ESP32-S3 + Edge Impulse ML + Sensor Package
```

### Sensor Packages
- `heat/` — Urban heat island monitoring (temperature, humidity, UV, pressure)
- `air/` — Air quality / wildfire smoke (PM2.5, VOC, CO, ozone)
- `flood/` — Flood early warning (water level, rain, soil moisture, barometric)
- `water/` — Water quality sentinel (turbidity, pH, DO, conductivity)

## Repo Layout
```
firmware/
  base/           # Shared: LoRa, power, LED, sensor interface, ML runner
  node/           # Node main firmware + config
  gateway/        # Gateway firmware + display + Firebase
  sensor_packages/  # One subdir per sensor variant
edge-impulse/     # Data collection tool + model configs
hardware/         # PCB, CAD, BOM files
docs/             # Architecture, LoRa protocol, sensor package docs
dashboard/        # Web dashboard (future)
```

## Key Conventions
- Firmware is Arduino C/C++ (.ino, .h, .cpp)
- Sensor packages implement the `SensorPackage` interface from `firmware/base/sensor_iface.h`
- Compile-time sensor selection via `#define SENSOR_PACKAGE_XXX` in `firmware/node/config.h`
- Edge Impulse models are exported as Arduino libraries into each sensor package's `model/` dir
- LoRa messages use `#<TYPE>,<fields...>*` framing (see docs/lora-protocol.md)
- Node uses FreeRTOS dual-core: Core 0 for comms/sensors, Core 1 for ML inference

## Building
- Use Arduino IDE or PlatformIO
- Target board: ESP32-S3 (node), Arduino Uno R4 WiFi (gateway)
- Each sensor package may require its own DFRobot or other sensor libraries
- Edge Impulse model library must be installed for the chosen sensor package

## Testing
- LoRa message serialization can be tested on host (no hardware needed)
- Sensor packages should compile independently against the base
- End-to-end: node → LoRa → gateway → display + Firebase
