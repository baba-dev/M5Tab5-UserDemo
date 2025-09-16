# Performance Guidelines

This guide documents the performance targets and diagnostic practices for the M5Tab5 User Demo so contributors can sustain a smooth user experience on constrained hardware.

## Frame-Rate Targets

| Scenario | Target FPS | Notes |
| --- | --- | --- |
| Idle dashboard | 30 FPS | Maintain smooth widget animations with minimal CPU wakeups. |
| Touch interactions | 45 FPS | Prioritize responsiveness during gesture handling and feedback. |
| Sensor visualization mode | 25 FPS | Balance refresh rate with data plotting complexity. |

Measure frame rate on-device using the built-in LVGL performance counters and log spikes above ±10% variance.

## Memory Limits

* **Total RAM budget**: 24 MB, including LVGL heap, network stack, and sensor buffers.
* **Display buffer**: Cap double-buffered LVGL draw buffers at 6 MB to leave headroom for application tasks.
* **Asset storage**: Keep in-memory icon and font caches below 4 MB; evict infrequently used assets aggressively.

Use the platform's heap tracing utilities to verify that steady-state usage remains below 80% of available RAM and that fragmentation stays under 10%.

## Profiling Tips

1. Enable LVGL's built-in profiler (`LV_USE_PERF_MONITOR = 1`) during development builds to capture CPU time per task.
2. Capture flame graphs with `perf` or `esp32-coredump` sampling when average frame time exceeds 33 ms.
3. Instrument MQTT handlers with timestamped logs to correlate network events with UI slowdowns.
4. Automate regression detection by tracking FPS and heap metrics in CI hardware-in-the-loop runs.

## Optimization Checklist

* Batch GPU-bound draw calls to minimize bus contention.
* Defer non-critical telemetry publishes when frame latency spikes.
* Compress large images and prefer indexed color palettes for UI assets.

For task breakdowns and prioritization guidance, see [TASKS.md](./TASKS.md).
