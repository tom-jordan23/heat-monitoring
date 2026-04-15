# Research Summary: Combined Fire Detection, Air Quality & Urban Heat Mapping Platform

## Project Goal

Build an open-source, solar-powered, LoRa-mesh-networked environmental monitoring platform that combines three functions in a single self-contained node:

1. **Fire detection** using multi-sensor fusion and on-device ML — works where smoke detection alone is unreliable
2. **Urban heat mapping** with distributed temperature, humidity, UV, and pressure sensors at neighborhood-level resolution — tracking not just current conditions but how vulnerability is shifting over time
3. **Air quality monitoring** to detect, classify, and track pollution sources over time

The platform operates without any existing infrastructure (no WiFi, cellular, or cloud required) and forms a self-organizing IoT mesh with edge analysis.

A core design premise: as climate change intensifies, existing systems — drainage, cooling infrastructure, air quality management, fire suppression capacity — will be pushed past their design limits. Communities and populations that cope today will become vulnerable tomorrow. Continuous distributed monitoring maps not just current hazards but marginal conditions, identifying where systems are approaching failure before they actually fail. Vulnerability maps are not static — they are shifting, and the data to track that shift doesn't exist at the resolution needed.

## Evaluated Base Projects

### Forest-Guard (github.com/MukeshSankhla/Forest-Guard)
**What it is:** ESP32-S3 sensor nodes + LoRa Meshtastic mesh + Arduino Uno R4 WiFi gateway + 3.5" TFT display + Firebase cloud. Detects gunshots and forest fires using Edge Impulse ML.

**Strengths:**
- Closest architecture to what we need (ESP32-S3, LoRa mesh, Edge Impulse, solar power, gateway+display)
- Dual-core FreeRTOS pattern (Core 0: sensors/comms, Core 1: ML inference)
- Working power management (solar + LiPo + TP4056)
- Custom PCB design with full schematics and BOM
- Environmental sensor (temp, humidity, UV, pressure, light) already integrated

**Weaknesses:**
- Tightly coupled to single use case (gunshot detection)
- No air quality sensors (only MEMS smoke)
- No data interoperability (Firebase only, no standards)
- No license stated — legal risk for derivative work
- Monolithic Arduino sketches, not modular
- Small community (32 stars), limited external validation

**Verdict:** Best architectural reference, but not forkable. Use as a design pattern source.

### Lumkani (lumkani.com)
**What it is:** Commercial fire detection system for informal settlements. Rate-of-rise temperature detection, wireless mesh with 60m radius, SMS alerts, paired with microinsurance.

**Strengths:**
- Most proven system for dense-environment fire detection (60,000+ devices, South Africa/Kenya/Bangladesh)
- Rate-of-rise detection solves THE fundamental problem (cooking smoke false alarms)
- 30-second alert cascade: detect → local 95dB alarm → mesh propagation → SMS/GPS to responders
- 94% of fires contained when system activated
- Community agent distribution and maintenance model
- Paired with microinsurance ($1.60/month) — sustainable business model

**Weaknesses:**
- Proprietary. No open-source code, hardware designs, or API
- Fire detection only — no air quality or heat mapping
- No ML/AI — uses fixed rate-of-rise threshold algorithm
- No environmental data output or interoperability

**Verdict:** Cannot be used as a base (proprietary), but its design principles should be foundational. Rate-of-rise detection, mesh alert cascade, and community deployment model are proven and should be adopted.

### Sensor.Community / AirRohr (sensor.community)
**What it is:** Citizen science air quality network. ESP8266 + SDS011/PMS5003 particle sensors + DHT22, WiFi-connected, pushes to open data platform.

**Strengths:**
- Largest citizen science air quality network in the world (35,000+ stations, 70+ countries)
- Extremely low cost (~$30-50 per node)
- MIT licensed, highly extensible
- Proven data pipeline and API
- Feeds into OpenAQ and other standard platforms
- Strong documentation and community

**Weaknesses:**
- WiFi only — no LoRa, no mesh, no off-grid capability
- ESP8266 too constrained for ML inference
- No fire detection capability
- Temperature/humidity is secondary (DHT22 only)
- Requires stable power and internet connection

**Verdict:** Excellent reference for air quality sensor integration and data interoperability standards. Not usable as a base due to WiFi-only architecture and no ML capability.

### CanAirIO (github.com/kike-canaries)
**What it is:** Open-source air quality monitoring from Colombia. ESP32 + multi-sensor support + mobile Bluetooth + fixed WiFi modes.

**Strengths:**
- Designed for low-cost, accessible deployment
- Excellent multi-sensor abstraction library (canairio_sensorlib) supporting Plantower, Sensirion, Honeywell, Nova sensors
- Dual deployment: mobile (Bluetooth+phone) and fixed (WiFi)
- Very low cost ($50-100)
- GPL-3.0 licensed, active community
- Good Android app for citizen science

**Weaknesses:**
- No LoRa support
- No fire detection
- No ML/inference capability
- Heat mapping not a focus
- GPL-3.0 copyleft may constrain future licensing

**Verdict:** The canairio_sensorlib multi-sensor abstraction is worth studying. Community engagement model is strong. Not usable as base due to no LoRa and no ML.

### Air Quality Station (airqualitystation.github.io)
**What it is:** Early-stage project to transmit air quality data via Meshtastic LoRa mesh. Sensirion SEN54/SEN55 sensors.

**Strengths:**
- Meshtastic LoRa mesh — same networking approach we need
- Sensirion SEN54/55 (PM, VOC, humidity, temp in one module)
- ESP32-S3 and nRF52 support
- Off-grid capable

**Weaknesses:**
- Very early stage, not production-ready
- No fire detection
- No ML/inference
- Limited documentation and community

**Verdict:** Confirms the Meshtastic + air quality sensor approach is viable. Too early to build on.

### Heltec BME688 LoRa Wildfire Detection
**What it is:** Prototype using Heltec ESP32 V3 boards + BME688 gas sensor + Meshtastic mesh for wildfire detection in Honduras.

**Strengths:**
- BME688 detects VOCs, CO, temperature, humidity, pressure — relevant to both fire and air quality
- Meshtastic mesh networking
- Real-world deployment (La Tigra National Park, Honduras)
- BSEC ML model training planned for clean air vs smoke classification

**Weaknesses:**
- Prototype only, not production-ready
- High standby power (98mA) — poor battery life
- Single sensor (BME688 alone insufficient for reliable fire detection)
- No community alerting mechanism

**Verdict:** Validates BME688 as a useful sensor for fire+air quality overlap. Power consumption is a cautionary tale.

## Data Interoperability Ecosystem

### Where our data should flow:

| Destination | Format | Purpose |
|---|---|---|
| **OpenAQ** | OpenAQ JSON API | Global air quality visibility, research correlation |
| **Sensor.Community** | Luftdaten JSON | Join the largest citizen science network |
| **OpenStreetMap** | `man_made=monitoring_station` tags | Map sensor locations in community and global mapping ecosystems |
| **OGC SensorThings API** | REST JSON + MQTT | Standardized IoT sensor interoperability |
| **HDX (Humanitarian Data Exchange)** | GeoJSON / CSV | Publish vulnerability indices for humanitarian coordination |
| **HOT Tasking Manager** | GeoJSON overlays | Inform where mapping campaigns should focus |

### Key standard: OGC SensorThings API
The OGC SensorThings API is the recommended middleware format. It standardizes heterogeneous sensor data into a consistent REST/MQTT interface that humanitarian and environmental platforms can consume.

## Recommended Sensor Stack

### Combined multi-hazard node (all three functions in one device):

| Function | Sensor | Model | Interface | Cost | Why |
|---|---|---|---|---|---|
| **Fire: rate-of-rise** | Fast-response thermistor | NTC 10K or DS18B20 | Analog/1-Wire | $1-3 | Lumkani-proven approach; measures dT/dt not smoke |
| **Fire: confirmation** | Gas sensor (CO + VOC) | BME688 | I2C | $20-40 | CO rises in fires; VOCs from burning materials; also serves air quality |
| **Fire: confirmation** | Optical particle | Plantower PMS5003 | UART | $15-25 | PM2.5 spike confirms fire; also serves air quality |
| **Air quality: particles** | PM2.5/PM10 | Plantower PMS5003 | UART | (shared above) | Industry standard, widely calibrated, feeds Sensor.Community |
| **Air quality: gases** | VOC + CO + humidity | Bosch BME688 | I2C | (shared above) | All-in-one gas + environmental; AI-capable |
| **Heat mapping: temp/humidity** | Environmental | BME688 or DFRobot SEN0500 | I2C | (shared above) | Heat index calculation needs temp + humidity |
| **Heat mapping: UV** | UV intensity | VEML6075 or LTR390 | I2C | $3-5 | UV exposure component of heat stress |

**Key insight:** The BME688 and PMS5003 serve double/triple duty across all three functions. A combined node needs only 3-4 sensors to cover all three goals.

### Estimated per-node cost: $120-180 complete
- ESP32-S3 module: $5-8
- LoRa module (RP2040 SX1262): $25-35
- BME688: $20-40
- PMS5003: $15-25
- UV sensor: $3-5
- Fast thermistor: $1-3
- Piezo buzzer (local alert): $1-2
- Solar panel (70x70mm): $10-15
- LiPo battery (1500mAh): $5-10
- TP4056 + regulator: $3-4
- PCB + enclosure: $20-30

### Gateway: $150-300
- Arduino Uno R4 WiFi (or ESP32 with WiFi): $40-50
- TFT display: $30-50
- LoRa receiver module: $25-35
- Enclosure + power: $30-50

## Recommended Architecture

### Why fresh scaffold (not a fork):

1. **No single project covers all three goals** — the landscape splits between fire detection (Forest-Guard) and air quality (Sensor.Community/CanAirIO), with nothing combining both
2. **Lumkani's proven design can't be forked** — it's proprietary, but its principles are well-documented
3. **Data interoperability is a first-class requirement** — none of the existing projects support OGC SensorThings, OpenAQ, or humanitarian mapping integration
4. **Combined multi-sensor inference** — no existing project does ML fusion across fire+air+heat sensors

### What to adopt from each project:

| Source | What to adopt |
|---|---|
| **Forest-Guard** | ESP32-S3 + Meshtastic LoRa architecture, dual-core FreeRTOS pattern, Edge Impulse integration, solar power management circuit, custom PCB approach |
| **Lumkani** | Rate-of-rise fire detection algorithm, 30-second mesh alert cascade with local buzzer, community deployment model, microinsurance pairing concept |
| **Sensor.Community** | PMS5003 integration patterns, data format for air quality reporting, calibration approaches, push-to-platform API pattern |
| **CanAirIO** | Multi-sensor abstraction library design (canairio_sensorlib), mobile data collection concept |
| **HOT/OSM ecosystem** | OGC SensorThings API output, OSM monitoring station tagging, GeoJSON vulnerability indices, HDX dataset publication |
| **NOAA heat campaigns** | 100m grid resolution for heat mapping, morning/afternoon/evening measurement protocol, heat index methodology |

### Multi-hazard ML inference strategy:

Instead of separate models per hazard, train a **single multi-output Edge Impulse model** on fused sensor data:

**Inputs (time-series window, ~2-5 minutes):**
- Temperature + dT/dt (rate of rise)
- Humidity + dH/dt
- PM2.5 + dPM/dt
- CO (from BME688 gas resistance)
- VOC index

**Outputs (multi-label classification):**
- Fire risk: none / possible / confirmed
- Air quality: good / moderate / unhealthy / hazardous
- Heat stress: normal / caution / danger / extreme

Over time, aggregated classification outputs build a picture of how often each node approaches its thresholds — how many hours per week a location spends in "caution" vs "normal," and whether that ratio is shifting. This is the marginal-to-unworkable signal: a location that was 95% "normal" two years ago and is now 80% "normal" is on a trajectory that matters for planning, even if it hasn't yet crossed into "danger."

**Cross-validation benefit:** A fire produces simultaneous rapid temp rise + PM2.5 spike + CO rise + humidity drop. This sensor fusion is far more reliable than any single sensor, and directly addresses the cooking-smoke false alarm problem (cooking produces smoke but NOT rapid temp rise + CO spike together).

### Alert cascade (adapted from Lumkani):

```
Sensor reads → ML inference (Core 1, continuous)
  ↓
FIRE detected (confidence > 90%):
  → Immediate: local 95dB buzzer ON
  → 0-30 seconds: LoRa mesh broadcast to all nodes in range
  → Neighbor nodes: activate their buzzers (cascade)
  → Gateway: SMS + GPS coordinates to responders
  → Gateway: Firebase/cloud push for dashboard

HEAT STRESS detected:
  → LoRa telemetry to gateway (no local alarm)
  → Gateway: update heat map display
  → Cloud: push to SensorThings API / HDX

AIR QUALITY event:
  → LoRa telemetry to gateway (no local alarm unless hazardous)
  → Gateway: update AQI display
  → Cloud: push to OpenAQ / Sensor.Community
```

## Adjacent Capabilities: Low-Cost Extensions Beyond the Three Core Functions

The core sensor stack (BME688 + PMS5003 + thermistor + UV sensor on ESP32-S3) already produces data that enables several additional capabilities — some at zero additional hardware cost, others for $1-5 per node. These were evaluated for humanitarian value, integration effort, and cost.

### Tier 1: Free or near-free (use existing sensors, software only)

**Weather / microclimate prediction — $0**
The BME688 already measures barometric pressure, temperature, and humidity. From these we can derive:
- **Dew point** (Magnus formula) — predicts fog, frost, condensation/mold risk in dwellings
- **Barometric pressure trends** — a 3-hour falling trend reliably predicts storms 2-4 hours ahead
- **Altitude estimation** — useful for understanding microclimates in hilly or varied terrain
- Integration: trivial (math on existing readings)

**Vector-borne disease risk index — $0**
Existing temperature + humidity data maps directly to mosquito breeding conditions. Optimal breeding: 20-30C with >55% RH. Combined with rainfall (if rain sensor added), this predicts dengue/malaria outbreak risk 2-4 weeks ahead. A simple composite score can trigger proactive measures — clearing standing water, distributing bed nets, scheduling larval surveys.
- Integration: trivial (scoring function on existing data)
- Humanitarian value: high — relevant anywhere mosquito-borne disease is a concern, from suburban Houston to rural Sub-Saharan Africa

**CO2 / indoor air quality estimation — $0**
The BME688's BSEC2 library provides estimated CO2, IAQ (Indoor Air Quality) index, and b-VOC (breath VOC) from its gas resistance sensor. Not scientifically accurate (±50-100 ppm), but useful for:
- Detecting overcrowded/poorly ventilated spaces
- Distinguishing cooking smoke from wildfire smoke (cooking = elevated CO2 + humidity; wildfire = CO + PM without CO2 spike)
- Integration: trivial (enable BSEC2 library, already using BME688)

**WiFi connectivity mapping — $0**
The ESP32-S3 can passively scan nearby WiFi networks and record signal strength (RSSI). Maps connectivity coverage — useful for identifying dead zones, planning access point placement, or verifying coverage claims.
- Integration: trivial (built-in WiFi scan API)

### Tier 2: Cheap additions ($1-5 per node, trivial-to-moderate integration)

**Rain detection — $2-4**
A simple resistive or tipping-bucket rain sensor on a GPIO/ADC pin. Enables:
- Flash flood early warning (rainfall accumulation + pressure drop + soil saturation)
- Water harvesting potential assessment
- Improves vector risk scoring (standing water after rain)
- Feeds into weather prediction (pressure trends + actual rainfall)
- Integration: trivial (GPIO interrupt or ADC)

**Soil moisture — $2-3**
Capacitive soil moisture sensor on ADC. The single strongest predictor of landslide risk on hillsides. Also enables:
- Flood risk assessment (saturated soil + rain = flash flooding)
- Agriculture and garden monitoring
- Foundation stability monitoring
- Integration: trivial (ADC input)
- Research backing: published studies validate capacitive sensors for cost-effective landslide early warning

**Noise level monitoring — $1-2**
INMP441 MEMS I2S microphone. The ESP32-S3 has native I2S support. Enables:
- Noise pollution mapping (correlates with stress, sleep disruption, cardiovascular disease)
- Could detect emergency sounds (sirens, structural collapse, rushing water)
- Environmental equity assessment (which areas suffer disproportionate noise exposure)
- Integration: moderate (I2S driver + audio buffering, can share Core 1 with ML)
- Reference: github.com/ikostoski/esp32-i2s-slm (mature ESP32 sound level meter)

**Light / lux sensor — $2-4**
BH1750 on I2C bus (we already have UV). Enables:
- Light pollution mapping (affects sleep, energy waste)
- Solar resource assessment for community microgrid planning
- Safety assessment (identifying poorly-lit areas)
- Integration: trivial (add to existing I2C bus)

**Accelerometer (seismic/structural) — $2-4**
ADXL345 or MPU6050 on I2C. Enables:
- Earthquake early warning (even 10 seconds of warning enables protective action)
- Building/structure vibration monitoring (detect instability before collapse)
- Landslide detection (ground acceleration changes when soil becomes unstable)
- Integration: moderate (I2C + FFT processing on Core 1)

**Human presence detection — $1-2 (PIR) or $8-15 (mmWave radar)**
PIR (HC-SR501) or LD2410 radar. Enables:
- Cooling center / shelter occupancy during heat events
- Evacuation monitoring during fires
- Space utilization tracking
- Integration: trivial (PIR = GPIO) or moderate (radar = I2C)

### Tier 3: Optional specialized additions ($5-15+)

**Water level sensor — $5-15**
Ultrasonic or capacitive water level sensor for flood-prone locations. Direct flash flood early warning for downstream areas. High value but only needed at specific deployment locations (streams, drainage channels, low points).

**Methane detection — $5-8**
MQ-4 sensor on ADC. Detects gas leaks and methane seepage near landfills or natural gas infrastructure. Cross-sensitive to other gases (H2, CO) so best used for trend detection and alerts rather than precise measurement.

**Power grid monitoring — $3-8**
AC voltage monitor module detects grid outages, voltage sags, and power quality. Nodes can broadcast outage alerts via mesh. Informs microgrid and solar capacity planning.

### Summary: Adjacent capability ROI

| Capability | Added Cost | Integration | Humanitarian Value | New Sensor? |
|---|---|---|---|---|
| Weather prediction | $0 | Trivial | High | No |
| Vector disease risk | $0 | Trivial | High | No |
| CO2/IAQ estimation | $0 | Trivial | Medium | No |
| WiFi coverage mapping | $0 | Trivial | Medium | No |
| Rain detection | $2-4 | Trivial | High | Yes |
| Soil moisture | $2-3 | Trivial | Very high (landslides) | Yes |
| Noise mapping | $1-2 | Moderate | Medium | Yes (MEMS mic) |
| Light/lux | $2-4 | Trivial | Medium | Yes |
| Seismic/structural | $2-4 | Moderate | High | Yes (accelerometer) |
| Human presence | $1-15 | Trivial-Moderate | Medium | Yes (PIR or radar) |
| Water level | $5-15 | Moderate | High (site-specific) | Yes |
| Methane | $5-8 | Trivial | Medium (site-specific) | Yes (MQ-4) |
| Power grid | $3-8 | Moderate | Medium | Yes |

### Recommendation: Design for extensibility, deploy in tiers

The four $0-cost capabilities (weather, vector risk, CO2/IAQ, WiFi mapping) should be included in the base firmware from day one — they're pure software on existing sensors.

Rain and soil moisture ($2-7 combined) should be standard optional headers on the PCB — they're trivially integrated and address landslide/flood risk, one of the most common and deadly weather hazards globally.

Everything else should be supported as optional modules: define the interface, reserve the GPIO/I2C/ADC pins, but don't require the hardware. Nodes deployed on hillsides get accelerometers; nodes near streams get water level sensors; nodes in shelters get PIR sensors.

This "base + optional extensions" approach keeps the core node cost at $120-180 while enabling $5-15 per node additions for site-specific hazards.

## Humanitarian Demand Side: What Organizations and Communities Actually Need

Vulnerable populations exist in every society — and climate change is creating new ones. Systems that currently manage heat, flooding, air quality, and fire risk were designed for historical climate conditions. As those conditions shift, the systems don't fail all at once — they degrade, becoming marginal before they become inadequate. The organizations and communities that need environmental data share a common problem: they can't see this degradation happening until it's too late.

Research into WHO, UNHCR, OCHA, C40 Cities, the Sendai Framework, and municipal/regional governments reveals common demand-side patterns:

### Key finding: organizations want event detection, not just averages
- Acute air pollution **spikes** (not just annual PM2.5 averages)
- Heat wave **intensity in real time** (not just temperature records)
- Flash flood **warnings** (not just rainfall probability)
- Water contamination **episodes** (not just monthly sampling)

### Spatial resolution: hyperlocal is the gap
- Current monitoring: 1-2 stations per city, often 10-100km from vulnerable communities
- Needed: neighborhood-level (100m-1km resolution)
- Our platform addresses this directly with distributed nodes — whether the gap is a suburban neighborhood between weather stations or an entire district with no monitoring at all
- This is a universal problem: heat islands in Phoenix, wildfire smoke corridors in suburban California, flood-prone neighborhoods in Houston are all under-monitored
- Critically, coarse monitoring masks marginal conditions. A city-wide average that reads "moderate" can hide neighborhoods already at their limit — and those are the places that will fail first as conditions worsen

### Data standards humanitarian and environmental platforms expect
- **JSON API** for real-time feeds to early warning systems
- **OGC SensorThings API** for IoT interoperability
- **GeoJSON** for mapping platforms (OSM, municipal GIS, HOT Tasking Manager)
- **HXL (Humanitarian Exchange Language)** tagged CSV for HDX/OCHA integration

### Trigger-based automation is the frontier
The early warning / early action model uses pre-agreed thresholds to automatically trigger responses:
- Sensor data crosses threshold → trigger fires → pre-positioned response activates
- Example: Heat index >35C for 3+ days → automatic alerts to outdoor workers, cooling center activation
- Example: PM2.5 >200 µg/m3 → school outdoor activity restrictions, health advisories
- In humanitarian contexts, OCHA's EWEA model uses this to automatically release funding — every $1 spent on anticipatory action saves $34 in emergency response (Nepal study)

Beyond immediate triggers, continuous monitoring enables **trend-based early warning** — detecting when a system is approaching its capacity limit before it actually fails. A neighborhood that hits dangerous heat index 3 days per summer today may hit it 15 days per summer in five years. That trend, visible in longitudinal sensor data, is an actionable signal for infrastructure investment, policy change, or community adaptation — before the crisis arrives.

### SDG monitoring alignment
Our platform directly supports measurement of:
- **SDG 3.9.1**: Mortality from air pollution (hyperlocal PM2.5 data)
- **SDG 11.6.2**: Annual mean PM2.5 in cities (neighborhood-level data that fills gaps in every city, not just developing ones)
- **SDG 13.1.1**: Deaths from disasters (early warning reduces these)
- **Sendai Framework Target D**: Local DRR strategy implementation

Longitudinal data from distributed sensors is especially valuable for SDG tracking because it reveals *trajectory*, not just current state. A city meeting SDG 11.6.2 today but trending toward failure is a different policy problem than one that's been stable for a decade — and you can't distinguish the two without continuous hyperlocal data.

### Parametric insurance opportunities
Sensor data enables threshold-triggered insurance products:
- Heat stress income protection (outdoor workers, triggered by heat index threshold)
- Flood property protection (triggered by rainfall + water level thresholds)
- Air quality health intervention funds (triggered by PM2.5 hazardous levels)
- Lumkani has already proven fire microinsurance paired with sensors at scale

## Development Phases

### Phase 1: Platform foundation
- Fresh repo scaffold with PlatformIO (not Arduino IDE — enables CI/testing)
- MIT license
- Base firmware: ESP32-S3 dual-core, LoRa Meshtastic comms, power management
- Combined sensor driver: BME688 + PMS5003 + thermistor + UV
- Enable BSEC2 library for free CO2/IAQ estimation
- Implement derived calculations: dew point, weather prediction from pressure trends
- Sensor reading and telemetry (no ML yet)
- Gateway: receive and display telemetry

### Phase 2: Fire detection
- Implement rate-of-rise algorithm (threshold-based, like Lumkani)
- Add local buzzer and mesh alert cascade
- Collect training data for Edge Impulse fire model
- Train and deploy multi-sensor fire classification model
- Test false positive rate against cooking, candles, sun exposure

### Phase 3: Air quality + heat mapping + vector risk
- Calibrate PMS5003 against reference monitors
- Implement AQI calculation (EPA standard)
- Implement heat index (Rothfusz regression) and heat vulnerability index
- Implement vector-borne disease risk scoring
- WiFi connectivity mapping (passive scan)
- Add data export: SensorThings API, OpenAQ format, Sensor.Community format
- Build heat vulnerability map output (100m grid GeoJSON)

### Phase 4: Humanitarian integration + dashboard
- OSM integration: tag sensor locations as monitoring stations
- HDX publication: heat and environmental vulnerability indices as datasets
- HXL-tagged data exports for OCHA compatibility
- HOT Tasking Manager: vulnerability data informs mapping priorities
- EWEA trigger integration: define thresholds for anticipatory action
- Web dashboard with map overlay

### Phase 5: Extended sensing + deployment
- PCB design with optional extension headers (rain, soil moisture, accelerometer, etc.)
- Rain + soil moisture as standard optional modules
- Seismic/structural monitoring for hillside deployments
- 3D-printed weatherproof enclosure (IP67)
- Deployment guides (urban neighborhoods, rural properties, field operations, humanitarian response)
- Explore parametric insurance partnerships (fire, heat, flood triggers)
