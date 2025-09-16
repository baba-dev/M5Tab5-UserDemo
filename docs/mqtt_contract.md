# MQTT contract

Canonical topic and payload definitions for the Tab5 firmware and Home Assistant bridge.
All payloads are UTF-8 JSON documents unless noted otherwise.

## Topic map

- `tab5/${device_id}/status` (Device → Broker): Online heartbeat with firmware info (retained).
- `tab5/${device_id}/telemetry` (Device → Broker): Batched sensor and input events.
- `tab5/${device_id}/media` (Device → Broker): Currently playing track metadata.
- `tab5/${device_id}/command` (Broker → Device): Command payloads for scenes and device levels.
- `tab5/${device_id}/ack` (Device → Broker): Command acknowledgements with status text.
- `homeassistant/light/${light_id}/config` (Device → Broker): MQTT discovery payload for lights.
- `homeassistant/sensor/${sensor_id}/config` (Device → Broker): Discovery payload for sensors.
- `homeassistant/button/${button_id}/config` (Device → Broker):
  Discovery payload for control buttons.

`${device_id}` is derived from the ESP32 MAC (e.g. `tab5-AC67B2`). All discovery topics comply with
Home Assistant MQTT Discovery and are published with the retained flag set.

## Payload schemas

### Status

```json
{
  "firmware": "1.2.3",
  "idf": "v5.5",
  "uptime_s": 12345,
  "heap_free": 247398,
  "wifi_rssi": -51,
  "charging": true,
  "battery_percent": 82
}
```

### Telemetry

An array so the firmware can publish multiple sensor readings in one message.

```json
[
  {
    "metric": "temperature_c",
    "value": 23.8,
    "ts": "2025-02-10T06:42:30Z"
  },
  {
    "metric": "touch",
    "value": {
      "x": 540,
      "y": 280,
      "gesture": "tap"
    },
    "ts": "2025-02-10T06:42:30Z"
  }
]
```

Each item must include `metric`, `value`, and ISO-8601 `ts`. Touch and IMU events place structured
values in `value`.

### Media metadata

```json
{
  "source": "living_room_sonos",
  "state": "playing",
  "title": "Astral Plane",
  "artist": "Valerie June",
  "album": "Astral Plane",
  "art_url": "https://example.com/astral.png"
}
```

### Command

```json
{
  "id": "d204f812",
  "type": "set_light",
  "target": "light.living_room_lamp",
  "payload": {
    "state": "on",
    "brightness": 180
  }
}
```

Recognised `type` values today include:

- `set_light`
- `set_fan`
- `set_media`
- `navigate_page`
- `trigger_scene`
- `set_brightness`
- `set_volume`
- `set_preference`

The firmware should reject unknown types.

### Ack

```json
{
  "id": "d204f812",
  "result": "ok",
  "error": null
}
```

`result` is either `"ok"` or `"error"`. When `result` is `"error"`, populate `error` with a short
detail string.

## Discovery payloads

Discovery documents follow the standard Home Assistant schema. Shared keys:

- `availability_topic`: `tab5/${device_id}/status`
- `payload_available`: `"online"`
- `payload_not_available`: `"offline"`

Example for a light entity the UI can control:

```json
{
  "name": "Living Room Lamp",
  "unique_id": "tab5-${device_id}-light-living-room",
  "command_topic": "tab5/${device_id}/command",
  "state_topic": "homeassistant/light/living_room/state",
  "schema": "json",
  "brightness": true,
  "supported_color_modes": ["brightness"],
  "device": {
    "identifiers": ["tab5-${device_id}"],
    "manufacturer": "M5Stack",
    "model": "Tab5",
    "sw_version": "1.2.3"
  }
}
```

When a command is executed the firmware publishes a fresh state snapshot to the matching HA state
topic so dashboards stay in sync.
