# MQTT Contract

This document summarizes the MQTT interface expectations for the M5Tab5 User Demo so firmware, mobile, and cloud teams can exchange telemetry and commands consistently.

## Topic Names

| Topic | Direction | Description |
| --- | --- | --- |
| `device/{deviceId}/status` | Device → Cloud | Periodic heartbeat including connectivity and firmware metadata. |
| `device/{deviceId}/telemetry` | Device → Cloud | Sensor readings such as temperature, humidity, and button state. |
| `device/{deviceId}/command` | Cloud → Device | Downstream instructions for UI updates, configuration changes, or resets. |
| `device/{deviceId}/ack` | Device → Cloud | Acknowledgements for command execution with success and error codes. |

All topics are namespaced per device using the stable `deviceId` assigned during provisioning. Avoid sharing topics between devices to preserve isolation and traceability.

## Payload Schemas

* **Status**: JSON object with `uptimeSeconds`, `firmwareVersion`, and `batteryPercent` fields.
* **Telemetry**: JSON array of objects with `metric`, `value`, and `timestamp` (ISO-8601) fields. This allows batching multiple readings per publish to reduce overhead.
* **Command**: JSON object specifying `type`, optional `parameters`, and `commandId` that the device must echo in the acknowledgement.
* **Acknowledgement**: JSON object with `commandId`, `result` (`"ok"` or `"error"`), and optional `errorDetail` string.

Where feasible, validate payloads against JSON Schema definitions embedded in integration tests to prevent regressions.

## QoS Expectations

* **Status**: QoS 0 is acceptable because the payload repeats frequently.
* **Telemetry**: QoS 1 ensures reliable delivery of sensor data while balancing latency.
* **Command**: QoS 1 is recommended so devices do not miss downstream instructions.
* **Acknowledgement**: QoS 0 is sufficient because command retries trigger additional acknowledgements.

Retained messages are only enabled for `device/{deviceId}/status` so the cloud can immediately assess last-seen device state.

## Operational Guidance

* Batch publishes to stay within broker rate limits while keeping telemetry latency under 5 seconds.
* Leverage shared subscriptions for cloud workers processing telemetry at scale.
* Monitor broker metrics (connection count, inflight messages) to catch backpressure early.

For roadmap alignment and upcoming milestones, see [TASKS.md](./TASKS.md).
