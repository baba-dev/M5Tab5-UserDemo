# M5Tab5 Home Control Tablet

[![IDF Build](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/idf-build.yml/badge.svg)](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/idf-build.yml)
[![Lint](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/lint.yml/badge.svg)](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/lint.yml)
[![Release Please](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/release-please.yml/badge.svg)](https://github.com/baba-dev/M5Tab5-UserDemo/actions/workflows/release-please.yml)
[![Web Flash](https://img.shields.io/badge/Web%20Flash-ESP%20Web%20Tools-38bdf8)](https://baba-dev.github.io/M5Tab5-UserDemo/flash.html)

**M5Stack Tab5 + ESP-IDF + LVGL** → a full-featured **Home Assistant control tablet**, Frigate viewer, and sensor bridge.

---

## 🌟 Project Overview

This project extends the official [M5Tab5 User Demo](https://docs.m5stack.com/en/esp_idf/m5tab5/userdemo) firmware into a **beautiful, animated home control panel**.

## Design & Docs

Links to key design and documentation resources:
- [`docs/TASKS.md`](docs/TASKS.md): prioritized roadmap for milestone planning.
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md): firmware structure and runtime data flow.
- [`docs/ASSETS.md`](docs/ASSETS.md): asset pipeline expectations and QA checklist.
- [`docs/AI_GUIDELINES.md`](docs/AI_GUIDELINES.md): workflow rules for AI-driven contributions.

- **Navigation Rail Preview**
  - Persistent left rail with icon buttons for ESP32P4 demo, Rooms, Frigate Security, Local Climate Station, and TV Controls.
  - Tap a destination to reveal a full-bleed gradient backdrop (`linear-gradient(90deg, #fcff9e 0%, #c67700 100%)`).
  - Cards, buttons, and title bars share softened 16 px radii and shadows to match the hardware aesthetic.

- **First Page: Default Demo for Tab5**
  The default user demo for testing out Tab5 features & control.

- **Second Page: Landscape-first dashboard (1280×720)**  
  At-a-glance view of lights, climate, security, and media from Home Assistant.

- **Third Page: Frigate Notifications+ Cameras**
Shows last detection in a intelligently parsed format along with camera name, time and object.
  - Live local stream (HLS→MJPEG→snapshot fallback)  
  - Swipe camera carousel  
  - Event timeline with thumbnails & PiP viewer


- **Fourth Page: Sensors Data, Visualizations, Charts**
Shows Sensors stats from Home Assistant
  - Nice Charts for history.  
  - More details and parsed sensor data.
 
    
- *Features List:*  

- **Home Assistant Integration (MQTT Discovery)**  
Seamless integration with local Home Asssistant via MQTT.
  - Toggle lights, fans, switches with state sync & animated controls.
  - View real-time sensor states with charts.
  - Control climate, media players and Android TVs.  
  - Trigger scenes and automations.
  
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

## 🧹 Formatting & Git Hooks

Install the repo's clang-format hook locally with [pre-commit](https://pre-commit.com/):

```bash
pip install pre-commit  # once per machine
pre-commit install
```

Running `pre-commit install` configures Git to automatically run clang-format on
changed files before each commit.

For manual formatting or static analysis runs, use the consolidated helper in
`tools/clang_tools.sh`:

```bash
./tools/clang_tools.sh format   # clang-format across tracked sources
./tools/clang_tools.sh tidy     # clang-tidy using compile_commands.json
./tools/clang_tools.sh fix      # format + clang-tidy --fix
```

---

## 📂 Repository Structure

The repo base is forked from default demo of M5Stack Tab5.
Our custom data is in the following folders:
- docs/ ← wireframes, AI guides, design tokens
- custom/ui/ ← LVGL UI modules
- custom/integration/ ← MQTT + HA + Frigate glue code
- custom/platform/ ← Display, input, audio, power drivers
- custom/assets/ ← Shared UI assets (fonts, icons, future binaries)

> See [`docs/wireframes.md`](docs/wireframes.md) for the low-fi design spec and [`docs/AI_Codex_Guide.md`](docs/AI_Codex_Guide.md) for AI coding guidelines.

## 🖼️ Visual Styling at Runtime

- Custom home screens render a built-in gradient background, so no wallpaper assets are flashed or loaded from storage.

## 🔧 Prerequisites

- **ESP-IDF v5.4.2** (tested with the ESP32-P4 target)
- **Python 3.8+** (for build scripts)
- Toolchain installed per [Espressif instructions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/get-started/)
- **libusb-1.0 runtime** (required by OpenOCD). On Debian/Ubuntu install via
  `sudo apt-get install libusb-1.0-0`; on macOS use `brew install libusb`.

> Tested with ESP-IDF v5.4.2 for esp32p4; newer releases may require additional fixes.

Optional:
- Mosquitto broker + Home Assistant for end-to-end testing
- Frigate (or any RTSP/MJPEG camera source)

---

## 🛠️ Build Instructions

1. **Clone the repo**

        git clone https://github.com/baba-dev/M5Tab5-UserDemo.git
        cd M5Tab5-UserDemo

2. **Select the Tab5 platform**

        cd platforms/tab5

3. **Set ESP32-P4 target**

        idf.py set-target esp32p4

        Only required once per build directory unless you clean the build output.

4. **Build firmware**

        idf.py build

5. **Configure (optional)**

        idf.py menuconfig

        Set Wi-Fi SSID & password (or flash once and store in NVS).

        Set MQTT broker URI & credentials.

### Web Flash

Every release publishes a browser flasher at **GitHub Pages** (Settings → Pages).
Open: `https://baba-dev.github.io/M5Tab5-UserDemo/flash.html`

This uses **ESP Web Tools** to flash `firmware.bin` described by `manifest.json`.
(Install button is loaded via the official script CDN.)

## 🚀 Flash & Monitor

Connect Tab5 over USB-C. Run the following from `platforms/tab5/`.

Flash:

`idf.py -p /dev/ttyACM0 flash
`

Monitor logs:

`idf.py monitor
`

Exit with Ctrl+].

### Wi-Fi remote firmware

ESP-IDF 5.4.2 bundles the supported ESP32 Wi-Fi remote under
`$IDF_PATH/components/espressif__esp_hosted/slave`. Flash that image once per
Tab5 so the SDIO link comes up reliably:

1. `cd $IDF_PATH/components/espressif__esp_hosted/slave`
2. `idf.py set-target esp32`
3. `idf.py build`
4. `idf.py -p <remote_port> flash`

The build output produces `build/network_adapter.bin` if you prefer to flash via
`esptool.py`. Shipping units use `CONFIG_SLAVE_IDF_TARGET_ESP32`, so the ESP32-C6
firmware is no longer stored in this repository. Host any experimental C6 image
as an external download and link it from the docs instead of committing binaries.


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

### UI golden snapshots
Run `scripts/update_goldens.sh` after UI changes to refresh the LVGL golden screenshots used by snapshot tests.

------------


## 🤝 Contributing

- Follow docs/AI_Codex_Guide.md for consistent code style and PR scope.

- Open small PRs for each feature (UI screen, component, or binding).

- Include screenshots or short videos of the UI changes in PR descriptions.

- Keep design tokens and wireframes up to date when introducing visual changes.
