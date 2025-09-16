# Architecture Overview

This document describes how the custom Tab5 firmware is structured so
contributors can extend it without breaking modular boundaries or performance
assumptions.

## System boundaries

- **Hardware**: M5Stack Tab5 (ESP32-P4, GT911 touch, ES8388 audio codec, IPS LCD
  1280×720).
- **Frameworks**: ESP-IDF 5.x for runtime, LVGL for UI composition, MQTT for
  Home Assistant connectivity.
- **Upstream code**: The base demo from M5Stack lives under `app/`; custom
  features must reside in `custom/` to keep merges with upstream simple.

## Firmware layout

- `custom/ui/`: screen implementations, shared widgets, and theme tokens.
- `custom/integration/`: MQTT, Home Assistant, and Frigate bindings plus mock
  providers.
- `custom/platform/`: display init, touch, audio, power management, and
  diagnostics.
- `custom/assets/`: RGB565 images, fonts, and other binary resources registered
  with LVGL.
- `docs/`: source-of-truth specifications for layout, tokens, contracts, and AI
  workflows.

Place cross-cutting utilities (logging helpers, configuration structs) in their
own subdirectory under `custom/` so they can be reused without polluting vendor
code.

## Runtime architecture

1. **Boot sequence**: `app_main` initializes ESP-IDF services, then calls into
   `custom::platform` to configure display buffers, touch, Wi-Fi, and the audio
   codec.
2. **UI bring-up**: `custom::ui::Init()` registers styles from `ui_theme.h`,
   builds persistent LVGL objects for each screen, and wires navigation events.
3. **Data bindings**: integration modules expose observer APIs that push state
   updates into the UI. UI interactions trigger command helpers that publish
   MQTT messages with optimistic feedback and rollback timers.
4. **Background services**: tasks under `custom::integration` manage MQTT
   sessions, Frigate stream lifecycles, and diagnostic telemetry. They
   communicate via queues or LVGL-safe callbacks to respect threading
   constraints.

## Configuration and secrets

- Store device credentials in NVS by default. Provide menuconfig options for
  initial provisioning but avoid hard-coding secrets in source control.
- Collect feature flags (mock data mode, diagnostics overlay, stream fallback
  overrides) in a shared configuration header so UI and integration code stay
  aligned.

## Performance considerations

- Follow the budgets listed in `docs/PERFORMANCE.md`. Create LVGL objects for
  all primary screens at startup to avoid heap churn during navigation.
- Use double-buffered rendering with carefully sized draw buffers (≤6 MB) and
  reuse animation timelines rather than re-creating them per interaction.
- Throttle background MQTT or Frigate work when the UI reports frame drops.

## Testing hooks

- Wrap MQTT access behind an abstraction so integration tests can swap in an
  in-memory broker.
- Expose metrics (FPS, heap, MQTT round-trip) through the diagnostics overlay to
  support manual and automated verification.
- Keep `idf.py build`, `idf.py lint`, and `npx markdownlint docs` passing before
  opening a PR.
