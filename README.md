# Community Sentinel

An open-source, solar-powered environmental monitoring platform that combines fire detection, air quality monitoring, and urban heat mapping in a single LoRa-mesh-networked node with on-device ML inference.

**Self-contained.** Each node runs on solar power with local storage and a 95dB buzzer — no external systems needed to detect and alert.

**No infrastructure required.** No WiFi, cellular, or cloud dependency. Nodes work out of the box in any environment — from a suburban backyard to a remote field site.

**IoT mesh with edge analysis.** Nodes form a LoRa mesh (Meshtastic) and run Edge Impulse ML models on-device. The network detects, classifies, and propagates alerts without a central server.

MIT License.

## Who This Is For

Community Sentinel is designed for anyone who needs distributed environmental monitoring and can't (or doesn't want to) depend on existing infrastructure:

- **Neighborhoods and local groups** — hyperlocal heat island and air quality data for your own block, not the nearest weather station 5 miles away
- **Schools and campuses** — outdoor air quality and heat safety monitoring across grounds
- **Farms, ranches, and rural properties** — fire detection and weather monitoring where cellular coverage is spotty or absent
- **Disaster response and field operations** — drop a mesh into an area and have monitoring running in minutes
- **Informal settlements and underserved communities** — environmental hazard monitoring where no infrastructure exists
- **Citizen science networks** — contribute data to OpenAQ, Sensor.Community, and other open platforms

The common thread: you want environmental awareness in places where traditional monitoring doesn't reach, doesn't exist, or isn't granular enough.

## Core Capabilities

### 1. Fire Detection
Rate-of-rise temperature detection (proven by Lumkani to avoid cooking-smoke false alarms) confirmed by CO/VOC + PM2.5 sensor fusion. Multi-sensor ML inference is far more reliable than any single sensor. Mesh-propagated alert cascade with local 95dB buzzer — neighbors are warned within 30 seconds.

### 2. Air Quality Monitoring
PM2.5/PM10 (Plantower PMS5003) + VOC/CO (Bosch BME688) for pollution mapping over time. Classifies pollution sources (cooking, traffic, industrial, wildfire smoke). Compatible with OpenAQ and Sensor.Community for global visibility.

### 3. Urban Heat Mapping
Temperature, humidity, UV, and barometric pressure for heat index calculation. 100m grid resolution GeoJSON output for integration with mapping platforms including OpenStreetMap and humanitarian systems (HOT/HDX).

## Extended Capabilities (from existing sensors, low/no additional cost)

The core sensor stack enables several additional functions at zero or minimal added cost:

| Capability | Added Cost | Source |
|---|---|---|
| Weather/storm prediction | $0 | Barometric pressure trends from BME688 |
| Dew point / mold risk | $0 | Derived from temperature + humidity |
| Vector disease risk scoring | $0 | Temperature + humidity predict mosquito breeding |
| CO2 / indoor air quality | $0 | BME688 BSEC2 library gas estimation |
| WiFi connectivity mapping | $0 | ESP32-S3 passive WiFi scan |
| Rain detection | $2-4 | Simple rain sensor on GPIO |
| Landslide/flood risk | $2-3 | Capacitive soil moisture sensor |
| Noise pollution mapping | $1-2 | MEMS I2S microphone |
| Seismic/structural monitoring | $2-4 | ADXL345 accelerometer |
| Light pollution / solar assessment | $2-4 | BH1750 lux sensor |

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│                      DATA PLATFORMS                       │
│  OpenAQ | Sensor.Community | OSM/HOT | HDX | SensorThings│
├──────────────────────────────────────────────────────────┤
│                     GATEWAY LAYER                         │
│  Arduino Uno R4 WiFi + LoRa RX + TFT Display + Cloud     │
├──────────────────────────────────────────────────────────┤
│                    LORA MESH NETWORK                      │
│             Meshtastic on RP2040 modules                  │
├──────────────────────────────────────────────────────────┤
│                      NODE LAYER                           │
│  ESP32-S3 + Edge Impulse ML + Combined Sensor Board       │
│                                                           │
│  Core: BME688 + PMS5003 + Thermistor + UV                │
│  Optional: Rain | Soil | Accel | Mic | Lux | Water Level │
└──────────────────────────────────────────────────────────┘
```

Each **node** combines fire detection, air quality, and heat mapping sensors on a single solar-powered board. On-device Edge Impulse ML performs multi-hazard classification. Nodes form a LoRa mesh for alert propagation — fire alerts cascade to neighboring nodes within 30 seconds.

A **gateway** collects data from the mesh, displays on a TFT screen, and optionally pushes to cloud platforms and open data ecosystems. The mesh operates fully without a gateway — the gateway adds connectivity, not dependency.

## Hardware (per node, ~$120-180)

| Component | Model | Cost |
|---|---|---|
| Microcontroller | ESP32-S3-WROOM-1 | $5-8 |
| LoRa radio | Waveshare RP2040 SX1262 (Meshtastic) | $25-35 |
| Gas/env sensor | Bosch BME688 (temp, humidity, pressure, VOC, CO) | $20-40 |
| Particle sensor | Plantower PMS5003 (PM1.0, PM2.5, PM10) | $15-25 |
| Fire detection | NTC fast-response thermistor (rate-of-rise) | $1-3 |
| UV sensor | VEML6075 or LTR390 | $3-5 |
| Alert | Piezo buzzer (95dB) | $1-2 |
| Power | 1500mAh LiPo + 70x70mm solar + TP4056 | $15-25 |
| Enclosure + PCB | Custom PCB, IP67 enclosure | $20-30 |

## Data Interoperability

When connected to a gateway, nodes can feed data to open platforms:

| Platform | Format | Purpose |
|---|---|---|
| OpenAQ | JSON API | Global air quality database |
| Sensor.Community | Luftdaten JSON | Citizen science AQ network |
| OpenStreetMap / HOT | `man_made=monitoring_station` | Community and humanitarian mapping |
| HDX (Humanitarian Data Exchange) | GeoJSON / HXL-tagged CSV | Humanitarian coordination |
| OGC SensorThings API | REST + MQTT | Standardized IoT interoperability |

## Repo Structure

```
firmware/
  base/              Shared code (LoRa, power, LED, sensor interface, ML runner)
  node/              Node firmware (ESP32-S3)
  gateway/           Gateway firmware (Arduino Uno R4 WiFi)
  sensor_packages/   Sensor driver code
edge-impulse/        Data collection tool and model configs
hardware/            PCB designs, 3D enclosures, BOMs
docs/                Research, protocol specs, architecture
dashboard/           Web dashboard (planned)
```

## Status

**Research phase (current):** Evaluating architecture, sensor selection, and integration points. See [docs/research-summary.md](docs/research-summary.md) for full analysis.

## Inspirations and References

- [Lumkani](https://lumkani.com/) — rate-of-rise fire detection at scale (60,000+ deployments)
- [Forest-Guard](https://github.com/MukeshSankhla/Forest-Guard/) — ESP32-S3 + LoRa + Edge Impulse architecture
- [Sensor.Community](https://sensor.community/) — citizen science air quality network (35,000+ stations)
- [CanAirIO](https://github.com/kike-canaries) — open-source air quality monitoring
- [NOAA urban heat campaigns](https://www.noaa.gov/) — distributed temperature sensing for urban heat islands
