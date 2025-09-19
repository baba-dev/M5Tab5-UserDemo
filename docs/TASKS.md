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
  - Outcome: add mock data sources for Rooms, CCTV, Weather, Media, and
    Settings pages under `custom/integration/` behind a build flag so each
    module in `custom/ui/pages/` has sample data.
  - Considerations: unblock UI work while MQTT is offline and document how to
    switch between real and mock providers.

### Milestone 1 — Page scaffolding

- **M1.1 — Rooms page scaffolding** (status: ☑)
  - Dependencies: M0.3.
  - Outcome: create persistent LVGL layout for `ui_page_rooms.*`, including the
    toolbar, wallpaper, and grid of `ui_room_card` widgets managed by
    `ui_root.c`.
  - Current state: `ui_page_rooms_create` renders three cards with static
    descriptors, plays intro animations, and emits toggle/sheet events backed by
    mock `rooms_state_t` data inside the module.
  - Considerations: keep card gestures coordinated with
    `custom/ui/pages/ui_rooms_model.*` when real data arrives.
- **M1.2 — CCTV page scaffolding** (status: ☑)
  - Dependencies: M0.3.
  - Outcome: build the CCTV layout in `ui_page_cctv.*` with toolbar controls,
    a `ui_room_card` wrapper for the live feed, and an events row prepared for
    camera history chips.
  - Current state: the page renders placeholder labels, hides unused toggles,
    and shows a stub "Live feed" container without switching logic or data.
  - Considerations: align toolbar button styles with shared assets from
    `custom/ui/ui_theme.h`.
- **M1.3 — Weather page scaffolding** (status: ☑)
  - Dependencies: M0.3.
  - Outcome: assemble the climate dashboard in `ui_page_weather.*`, featuring
    indoor/outdoor cards and a flexible forecast row that can accept dynamic
    metrics.
  - Current state: static strings populate each metric block and forecast item,
    and the card toggles/specs are hidden until bindings are available.
  - Considerations: confirm copy and iconography match `docs/wireframes.md`.
- **M1.4 — Media page scaffolding** (status: ☑)
  - Dependencies: M0.3.
  - Outcome: layout the media experience in `ui_page_media.*`, including now
    playing metadata, transport controls, quick scenes, and volume slider ready
    for callbacks.
  - Current state: album art, track labels, and transport buttons display
    placeholder content with no LVGL event handlers attached.
  - Considerations: ensure button spacing matches the reusable metrics from
    `custom/ui/ui_theme.h`.
- **M1.5 — Settings page scaffolding** (status: ☑)
  - Dependencies: M0.3.
  - Outcome: implement the settings hierarchy in `ui_page_settings.*` with
    connectivity tests, theme controls, network tools, OTA prompts, and backup
    actions surfaced through `ui_page_settings_actions_t`.
  - Current state: the page constructs all sections, exposes setters such as
    `ui_page_settings_set_connection_status`, and binds actions via
    `app_launcher/view`, but integration calls still return placeholder states
    when hardware services are offline.
  - Considerations: document the action wiring in `docs/ARCHITECTURE.md` when
    integration matures.

### Milestone 2 — Data plumbing

- **M2.1 — Rooms state pipeline** (status: ☐)
  - Dependencies: M1.1.
  - Outcome: move the static room descriptors and `INITIAL_STATE` out of
    `ui_page_rooms.c` into a provider so `ui_page_rooms_set_state` consumes data
    supplied by `custom/integration/`.
  - Current state: not started; the page seeds its own mock entities and ignores
    external updates.
  - Considerations: surface toggle events via a shared bus so integrations can
    publish commands.
- **M2.2 — CCTV event model** (status: ☐)
  - Dependencies: M1.2.
  - Outcome: define data structures for camera rotation and timeline chips,
    exposing setters on `ui_page_cctv.*` so integrations can drive the view.
  - Current state: not started; toolbar labels and chips are hard-coded.
  - Considerations: plan for Frigate clip metadata and snapshot fallbacks.
- **M2.3 — Weather and climate adapter** (status: ☐)
  - Dependencies: M1.3.
  - Outcome: add an adapter layer that formats indoor, outdoor, and forecast
    data for `ui_page_weather.*`, including unit conversions and icon mapping.
  - Current state: not started; the page renders placeholder Fahrenheit/Celsius
    values without conversion helpers.
  - Considerations: reuse formatting helpers across Settings where applicable.
- **M2.4 — Media control hooks** (status: ☐)
  - Dependencies: M1.4.
  - Outcome: attach LVGL callbacks in `ui_page_media.*` that emit play/pause,
    skip, volume, and quick-scene events to a media controller in
    `custom/integration/`.
  - Current state: not started; controls are visual only.
  - Considerations: coordinate icon/text updates when playback state changes.
- **M2.5 — Settings telemetry bridge** (status: ☐⚙)
  - Dependencies: M1.5.
  - Outcome: finish threading connection test results, OTA progress, and theme
    persistence through `custom/integration/settings_controller.*` so UI status
    pills stay in sync.
  - Current state: partially implemented; `SettingsController` posts events but
    lacks coverage for every action and has stubbed fallback messages when
    platform services are unavailable.
  - Considerations: audit async callbacks to ensure they survive suspend/resume
    of the worker thread.

### Milestone 3 — Home Assistant integration

- **M3.1 — MQTT bindings for Rooms** (status: ☐)
  - Dependencies: M2.1.
  - Outcome: subscribe to room sensor and light topics defined in
    `docs/mqtt_contract.md`, translate them into `rooms_state_t`, and publish
    toggle commands emitted by `ui_page_rooms`.
  - Current state: not started; Rooms still relies on compile-time mocks.
  - Considerations: debounce slider updates so MQTT traffic stays bounded.
- **M3.2 — Climate and weather ingestion** (status: ☐)
  - Dependencies: M2.3.
  - Outcome: map MQTT sensor payloads to the weather adapter and refresh indoor
    metrics alongside settings-managed preferences like units and themes.
  - Current state: not started; all weather values are placeholders.
  - Considerations: document required topics and units in
    `docs/mqtt_contract.md`.
- **M3.3 — Media and Settings integration** (status: ☐)
  - Dependencies: M2.4 and M2.5.
  - Outcome: hook media control events into Assist/Home Assistant transports and
    persist device settings (theme, brightness, updates) through MQTT or REST as
    defined in `custom/integration/`.
  - Current state: not started; media commands are local stubs and settings
    persistence runs on in-memory defaults.
  - Considerations: record integration quirks in `docs/ARCHITECTURE.md` and
    expand `docs/mqtt_contract.md` when new topics are introduced.

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
