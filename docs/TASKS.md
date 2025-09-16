# Task Roadmap

This roadmap sequences the work required to transform the stock M5Stack Tab5
demo into the Home Assistant control experience described in the wireframes. It
is written so humans and AI assistants can pick up a task, understand
prerequisites, and deliver a focused change.

## How to work through the backlog

- Start at the top of the milestone that is currently in progress.
- Break tasks into pull requests that stay below ~500 lines of diff.
- Keep documentation and source code changes in the same pull request when they
  are related.
- Update the status marker when a task is handed off between contributors.
- When a task exposes new constraints, add notes in the handoff bullet and
  mirror key findings in `docs/AI_Codex_Guide.md` or `docs/wireframes.md`.

## Milestones and tasks

### Milestone 0 — Foundations

- **M0.1 — Toolchain smoke test** (status: ☐)
  - Dependencies: none.
  - Outcome: confirm `idf.py build` succeeds before any modifications.
  - Considerations: capture environment quirks in `README.md`.
- **M0.2 — Credential strategy** (status: ☐)
  - Dependencies: M0.1.
  - Outcome: document how Wi-Fi, MQTT, and Assist credentials are stored (NVS vs
    compile time).
  - Considerations: update `docs/ARCHITECTURE.md` with the decision.
- **M0.3 — Mock providers** (status: ☐)
  - Dependencies: M0.1.
  - Outcome: add mock data sources for Home, Rooms, and Security screens under
    `custom/integration/` behind a build flag.
  - Considerations: unblock UI work while MQTT is offline.

### Milestone 1 — UI skeleton

- **M1.1 — Screen scaffolding** (status: ☐)
  - Dependencies: M0.3.
  - Outcome: create persistent LVGL objects for `ui_home.*`, `ui_rooms.*`,
    `ui_security.*`, and `ui_control_center.*` that match the layout spec.
  - Considerations: reuse placeholder widgets from `custom/ui/ui_components.*`.
- **M1.2 — Shared components** (status: ☐)
  - Dependencies: M1.1.
  - Outcome: implement cards, chips, sliders, tabs, and confirm sheets styled
    via `ui_theme.h`.
  - Considerations: centralize styles so future screens stay consistent.
- **M1.3 — Navigation and gestures** (status: ☐)
  - Dependencies: M1.1.
  - Outcome: wire the tab rail, edge swipes, and control center sheet
    interactions.
  - Considerations: include optimistic animations for taps and long presses.

### Milestone 2 — Feature scaffolding

- **M2.1 — Home bindings** (status: ☐)
  - Dependencies: M1.2.
  - Outcome: connect Home cards to mock providers with optimistic updates ready
    for MQTT swap.
  - Considerations: keep command helpers in `custom/integration/`.
- **M2.2 — Rooms interactions** (status: ☐)
  - Dependencies: M1.2.
  - Outcome: implement room detail updates, two-finger swipe navigation, and
    long-press sheets.
  - Considerations: validate gestures on hardware for reliability.
- **M2.3 — Security carousel** (status: ☐)
  - Dependencies: M1.2.
  - Outcome: build the camera carousel and event chips backed by mock Frigate
    data.
  - Considerations: respect the HLS → MJPEG → snapshot fallback described in
    the AI guide.

### Milestone 3 — Home Assistant integration

- **M3.1 — MQTT for Home and Rooms** (status: ☐)
  - Dependencies: M2.1 and M2.2.
  - Outcome: replace mock providers with real MQTT subscriptions and publish
    helpers.
  - Considerations: extend `docs/MQTT_CONTRACT.md` with payload schemas.
- **M3.2 — Frigate live view** (status: ☐)
  - Dependencies: M2.3.
  - Outcome: stream live video with fallback handling and update the event
    timeline from Frigate.
  - Considerations: measure frame times against `docs/PERFORMANCE.md`.
- **M3.3 — Control center commands** (status: ☐)
  - Dependencies: M2.1.
  - Outcome: publish brightness, volume, and toggle commands with rollback
    timers.
  - Considerations: log failures for debugging and expose status to the UI.

### Milestone 4 — Quality and automation

- **M4.1 — Diagnostics overlay** (status: ☐)
  - Dependencies: M3.1 through M3.3.
  - Outcome: surface FPS, heap usage, and LVGL draw time behind a debug toggle.
  - Considerations: explain activation steps in `README.md`.
- **M4.2 — Hardware regression suite** (status: ☐)
  - Dependencies: M3.x series tasks.
  - Outcome: create a smoke-test script or CI workflow that exercises MQTT
    flows and key UI paths.
  - Considerations: share setup instructions alongside the script.

Status legend: ☐ pending, ☐⚙ in progress, ☑ done.

## Definition of done reminders

- Match layout and tokens defined in `docs/wireframes.md` and
  `custom/ui/ui_theme.h`.
- Route telemetry and commands through helpers in `custom/integration/` (no
  broker logic in UI files).
- Run the build and lint commands listed in the root `AGENTS.md` before
  requesting review.
- Update README and relevant docs with new capabilities or constraints.

## Coordination tips

- Reserve feature branches or issues before starting large tasks to avoid
  overlap.
- Attach demo photos or short videos to pull requests so reviewers can evaluate
  animations.
- When handing off partially complete work, link the working branch and
  summarize progress here.
