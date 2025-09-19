M5Tab5 User Demo — AI Build Guide (for Codex & Co.)

Repo: baba-dev/M5Tab5-UserDemo
Device: M5Stack Tab5 (Landscape 1280×720)
UI Stack: LVGL
Integrations: Home Assistant (MQTT + Assist), Frigate

Source of truth for layout & interactions: docs/wireframes.md
AI execution rules: this file + docs/AI_GUIDELINES.md (if present)

0) Mission & Constraints
Mission

Implement the Rooms, CCTV, Weather, Media, and Settings experiences as per the low-fi wireframes.
Produce small, reviewable changes that run smoothly on device.

Hard Constraints (do not deviate)

Respect all tokens, spacing, and motion from docs/wireframes.md.

Landscape 1280×720; no other orientations.

Pre-create UI objects; avoid per-frame allocations.

Image assets must be pre-sized RGB565 (no runtime scaling).

Optimistic UI with visible confirmation or rollback on failure.

Keep interaction animations ≥50 FPS, ambient ≥30 FPS.

1) Files & Folders (expected structure)
/docs
  wireframes.md                ← low-fi spec (read first)
  AI_Codex_Guide.md            ← this file (you are here)
  AI_GUIDELINES.md             ← optional extra rules

/custom/ui
  ui_theme.*                   ← tokens (spacing, colors, radii, motion)
  ui_nav_rail.*                ← navigation rail & gestures
  ui_root.*                    ← screen container + page routing
  ui_wallpaper.*               ← shared wallpaper helper
  pages/
    ui_page_rooms.*            ← Rooms cards + detail events
    ui_page_cctv.*             ← CCTV view + event list
    ui_page_weather.*          ← Local climate metrics
    ui_page_media.*            ← Media controls & scenes
    ui_page_settings.*         ← Settings sliders + testers
    ui_rooms_model.*           ← shared Rooms state helpers
  widgets/                     ← shared cards/chips/sliders/toasts

/assets
  icons/                       ← 1x assets sized to final use, RGB565
  images/                      ← static backgrounds/thumbnails

/integration
  mqtt_contract.h              ← topic names & payload schemas (stubs OK)
  ha_bindings.*                ← glue for HA state/commands
  frigate_bindings.*           ← camera/event glue

/platform
  tab5_display.*               ← display init, LVGL init, buffers
  input_touch.*                ← GT911 gestures (tap, long-press, swipes)
  audio_codec.*                ← ES8388 (mic/speaker) control


If a file doesn’t exist yet, create it using this layout and keep changes scoped.

2) Design Tokens (use these names)

Re-use the exact token names from docs/wireframes.md. If missing, declare them in ui_theme.h using the same names:

Spacing: space-4, -8, -16, -24, -32

Radii: radius-20, radius-12

Elevation: elev-1, elev-2

Colors: surface-1, surface-2, text, muted, accent, warn, alert

Typography: title-lg, title-md, body, caption

Motion: dur-fast (90ms), dur-med (200ms), dur-slow (300ms), ease-out

Touch target: min 56×56 px

Do not invent new tokens, colors, or timings. If you need one, add it to docs/wireframes.md via PR first.

3) Interaction Contract (from wireframes)

Implement exactly these behaviors:

Rooms (ui_page_rooms.*)

- Grid of room cards populated from `ui_rooms_model.*`.
- Tap the toggle on a card to fire `UI_PAGE_ROOMS_EVENT_TOGGLE` with optimistic feedback.
- Long-press a card body to request the per-room sheet via `UI_PAGE_ROOMS_EVENT_OPEN_SHEET`.
- Cards surface temperature/humidity badges and availability state.

CCTV (ui_page_cctv.*)

- Primary camera card with Prev/Next controls, quality picker, and mute toggle shell.
- Live feed placeholder sized per spec; keep toggles hidden on camera/event cards.
- Event cards list recent clips; tapping should emit an open-clip signal when bindings arrive.

Weather (ui_page_weather.*)

- Living Room card shows indoor temperature, humidity, and HVAC mode metrics.
- Outdoor card surfaces raw sensor IDs with current readings.
- Forecast row presents 3 quick-glance day summaries with iconography.

Media (ui_page_media.*)

- Now Playing card renders album art, metadata, transport buttons, and a volume slider.
- Quick Scenes card exposes preset buttons for lighting/media moods.
- Hide unused default room-card specs/toggles when not applicable.

Settings (ui_page_settings.*)

- Connectivity testers display pill states (Online/Degraded/Offline) per status callbacks.
- Theme block combines dark-mode switch and theme dropdown; changes propagate via async payloads.
- Brightness slider updates the numeric pill; action buttons (OTA, diagnostics, logs)
  forward events.

Global

Swipe down on header → Control Center.

Edge swipe (right) → tab rail (if hidden).

Toast on success; visible error/rollback on failure.

4) Performance Budget & Checks

Target ≤16ms per frame for interactions.

Limit parallel tweens ≤6 per screen.

Preload & cache assets; no runtime scaling; reuse styles.

Keep heap/PSRAM headroom; expose a developer toggle for debug overlay (FPS, heap, LVGL draw time).

5) Home Assistant & Frigate (binding plan)

Provide a thin glue layer; UI should not embed MQTT details.

MQTT Contract (stub expectations)

State topics per entity (lights, fans, climate, sensors).

Command topics mirror state with clear payloads.

QoS conservative (0/1) depending on entity criticality.

Retained only for persistent state where safe (e.g., brightness).

Add/extend /integration/mqtt_contract.h with:

Topic strings (const) for each screen’s needs.

Payload schemas (brief comments).

Helper funcs for optimistic update + rollback timer.

Frigate

Provide functions to:

Get live stream URL (prefer local low-latency).

Fetch recent events (with label + time).

Fallback chain: HLS → MJPEG → snapshot (UI shows badge).

6) What to build first (task slices)

Skeleton screens (layout only)

- Stand up `custom/ui/ui_root.*` with nav rail scaffolding.
- Create empty `ui_page_rooms.*`, `ui_page_cctv.*`, `ui_page_weather.*`, `ui_page_media.*`,
  and `ui_page_settings.*` with layout placeholders using shared widgets.

Rooms iteration

- Build the room card grid with `ui_room_card` widgets and wire toggle/long-press events.
- Populate cards from `ui_rooms_model.*`; surface climate badges and availability state.

CCTV iteration

- Compose the live camera card with toolbar controls and video placeholder.
- Lay out event cards for recent clips; stub tap handlers for future bindings.

Weather iteration

- Add Living Room and Outdoor cards with metric stacks.
- Render the forecast row with icon + high/low pairs.

Media iteration

- Flesh out the Now Playing card (album art, metadata, transport controls, volume slider).
- Add Quick Scenes buttons and make them emit scene selection events.

Settings iteration

- Implement connectivity testers, theme toggle/dropdown, brightness slider, and action buttons.
- Expose async setters for status/theme/brightness updates.

Motion & feedback

- Add transitions for card/toolbars, slider feedback, and toast confirmations; cover error states.

Debug & Perf

- Hidden overlay; verify FPS/allocations.

Each step should be a small PR (see checklist below).

7) Pull Request Checklist (AI & humans)

 Implements a single wireframe section (name it)

 Uses tokens from ui_theme.h (no ad-hoc values)

 Zero per-frame allocations (explain any exceptions)

 Asset sizes pre-scaled RGB565 (list any added assets)

 Animations meet FPS targets (note observed FPS)

 Interactions match docs/wireframes.md

 MQTT/Frigate bindings use integration/* helpers

 Includes short video/screenshot for review

 Notes any TODOs or open questions

8) How to “think” with this repo (prompt Codex like this)

Use a standard preface when asking the AI for changes:

Context: You are working in baba-dev/M5Tab5-UserDemo.
Read docs/wireframes.md and docs/AI_Codex_Guide.md.
Build only what those files describe. Use LVGL on Tab5 at 1280×720 landscape.
Respect tokens in ui_theme.h. Keep changes minimal and isolated: prefer editing custom/ui/ui_root.*,
custom/ui/pages/ui_page_*.*, custom/ui/pages/ui_rooms_model.*,
custom/ui/widgets/*, or integration/*.
Preload RGB565 assets sized to display; no runtime scaling.
Use optimistic UI with rollback on timeout. Add a short note on performance and video of the result.

Example task prompts (no code requested here)

“Implement the Home screen card layout per docs/wireframes.md, using ui_components building blocks. Wire fake values via a mock provider; no MQTT yet. Add tap/expand animation and toasts per spec.”

“Add Control Center modal with brightness/volume and two toggles. Publish brightness as a HA light entity (topic stub), optimistic update + rollback.”

“Hook Security screen to Frigate bindings with fallback HLS→MJPEG→snapshot and show the ‘Low bandwidth’ badge when falling back.”

9) Decision rules (when unclear)

If wireframe and code disagree → wireframe wins.

If a token is missing → add it to ui_theme.h and propose the same in docs/wireframes.md.

If performance and fidelity conflict → prioritize performance (smooth > fancy).

Avoid adding new dependencies unless approved in PR discussion.

10) Definition of Done (per screen)

Layout matches wireframe (spacing, sizes, hierarchy).

All interactions implemented (tap/long-press/swipes/toasts).

Real-time state updates hooked (MQTT/Frigate), with optimistic UI + rollback.

Meets performance targets; debug overlay shows frame times under budget.

Edge cases handled: offline/no data/low bandwidth.

11) Known unknowns (track here)

Best-performing local stream type for Frigate on Tab5 GPU path.

HA theme sync contract (if adopted).

Voice pipeline (on-device vs HA Assist latency budget).

Keep this section updated in PRs so future AI runs have context.

Final note

This document + docs/wireframes.md are the contract. If you (AI) need to deviate, open a PR that first updates the docs, then the code.
