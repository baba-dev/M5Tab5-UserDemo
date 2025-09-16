# M5Tab5 Home Control Tablet

**M5Stack Tab5 + ESP-IDF + LVGL** → a full-featured **Home Assistant control tablet**, Frigate viewer, and sensor bridge.

---

## 🌟 Project Overview

This project extends the official [M5Tab5 User Demo](https://docs.m5stack.com/en/esp_idf/m5tab5/userdemo) firmware into a **beautiful, animated home control panel**.

## Design & Docs

Links to key design and documentation resources:
- [`docs/TASKS.md`](docs/TASKS.md)
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
- [`docs/ASSETS.md`](docs/ASSETS.md)

- **First Page: Default Demo for Tab5**
The default user demo for testing out Tab5 features & control.

- **Second Page: Landscape-first dashboard (1280×720)**  
  At-a-glance view of lights, climate, security, and media from Home Assistant.

- **Home Assistant Integration (MQTT Discovery)**  
Seamless integration with local Home Asssistant via MQTT.
  - Toggle lights, fans, switches with state sync & animated controls.
  - View real-time sensor states with charts.
  - Control climate, media players and Android TVs.  
  - Trigger scenes and automations.
  
- **Frigate + Cameras**
Shows last detection in a intelligently parsed format along with camera name, time and object.
  - Live local stream (HLS→MJPEG→snapshot fallback)  
  - Swipe camera carousel  
  - Event timeline with thumbnails & PiP viewer

- **Voice Control**  
Using Tab5 Mics & Speakers with HA
  - Wake-word detection on-device  
  - Audio routed through ES8388 mic/speaker  
  - Forward to Home Assistant Assist or external LLM

- **Tab5 Sensor Exposure**
Using stuff from base code, we get and publish these states to HA:
  - Battery voltage/current, charging state  
  - Touch, button, and IMU events  
  - Audio levels & speaker status  
  - Publish via MQTT for HA use

- **Control Center**  
  - Brightness slider (backlight light entity)  
  - Volume & mute  
  - Quick toggles: Do Not Disturb, Auto-brightness  
  - Sleep & restart controls

- **Developer Experience**  
  - Modular LVGL components (`ui_home`, `ui_rooms`, `ui_security`, `ui_control_center`)  
  - Design tokens & wireframes in `docs/` for AI/assistants  
  - CI-friendly (ESP-IDF build, firmware artifacts)  

---

## 📂 Repository Structure

The repo base is forked from default demo of M5Stack Tab5.
Our custom data is in the following folders:
- docs/ ← wireframes, AI guides, design tokens
- custom/ui/ ← LVGL UI modules
- custom/integration/ ← MQTT + HA + Frigate glue code
- custom/platform/ ← Display, input, audio, power drivers
- custom/assets/ ← Icons, backgrounds (RGB565, pre-sized)

> See [`docs/wireframes.md`](docs/wireframes.md) for the low-fi design spec and [`docs/AI_Codex_Guide.md`](docs/AI_Codex_Guide.md) for AI coding guidelines.

## 🔧 Prerequisites

- **ESP-IDF 5.x** (P4 target support required)
- **Python 3.8+** (for build scripts)
- Toolchain installed per [Espressif instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/get-started/)

Optional:
- Mosquitto broker + Home Assistant for end-to-end testing
- Frigate (or any RTSP/MJPEG camera source)

---

## 🛠️ Build Instructions

1. **Clone repo & submodules**

	git clone https://github.com/baba-dev/M5Tab5-UserDemo.git
   cd M5Tab5-UserDemo
   python fetch_repos.py  # fetch M5 & LVGL components

2. **Set ESP32-P4 target**

	idf.py set-target esp32p4

3. **Configure (optional)**

	idf.py menuconfig

	Set Wi-Fi SSID & password (or flash once and store in NVS).

	Set MQTT broker URI & credentials.

## 🚀 Flash & Monitor

Connect Tab5 over USB-C.

Flash:

`idf.py -p /dev/ttyACM0 flash
`

Monitor logs:

`idf.py monitor
`

Exit with Ctrl+].


------------

## 🌐 Home Assistant Setup

- Ensure Mosquitto or other MQTT broker is running.

- Enable MQTT Discovery in HA.

- After boot, entities (lights, switches, sensors) will appear automatically.

- Optionally configure automations to send control payloads back to Tab5 (e.g., set brightness, trigger scenes).

------------


## 🧪 Development Tips

- Use LVGL debug overlay (if enabled) to monitor FPS and heap usage.

- Check integration/mqtt_contract.h for topic map and extend as needed.

- Test camera performance with multiple stream types; pick best one for latency.

------------


## 🤝 Contributing

- Follow docs/AI_Codex_Guide.md for consistent code style and PR scope.

- Open small PRs for each feature (UI screen, component, or binding).

- Include screenshots or short videos of the UI changes in PR descriptions.

- Keep design tokens and wireframes up to date when introducing visual changes.
