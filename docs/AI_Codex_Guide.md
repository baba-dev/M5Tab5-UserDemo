M5Tab5 User Demo — AI Build Guide (for Codex & Co.)

Repo: baba-dev/M5Tab5-UserDemo
Device: M5Stack Tab5 (Landscape 1280×720)
UI Stack: LVGL
Integrations: Home Assistant (MQTT + Assist), Frigate

Source of truth for layout & interactions: docs/wireframes.md
AI execution rules: this file + docs/AI_GUIDELINES.md (if present)

0) Mission & Constraints
Mission

Implement the Home, Rooms, Security, and Control Center screens as per the low-fi wireframes. Produce small, reviewable changes that run smoothly on device.

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

/ui
  ui_theme.h                   ← tokens (spacing, colors, radii, motion)
  ui_components.*              ← shared cards/chips/sliders/toasts
  ui_home.*                    ← Home screen
  ui_rooms.*                   ← Rooms screen
  ui_security.*                ← Security (Frigate) screen
  ui_control_center.*          ← Control Center sheet
  ui_debug.*                   ← optional debug overlay (FPS, heap)

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

Home

Cards: tap → expand; long-press Quick Action → confirm sheet.

Status chips: navigate to relevant screen; anomaly colors: accent/warn/alert.

Rooms

List left, detail right; two-finger swipe changes room.

Long-press a light → per-entity detail sheet.

Security

Camera carousel + Live tile; event chips open clip/PiP.

Alert mode: frame screen in warn/alert, focus automatically.

Control Center

Swipe-down to open; brightness, volume, toggles; sleep action.

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

Create empty ui_home.*, ui_rooms.*, ui_security.*, ui_control_center.* with layout objects and placeholder components from ui_components.*.

Hook tab navigation & gestures.

Components

Shared Card, Chip, Slider, Toast, Tabs, Confirm Sheet—styled via ui_theme.h.

Bindings

Wire Home card values to mock data → then swap to MQTT/Frigate bindings.

Motion & feedback

Add expand/contract, color pulse, toast confirmations; error states.

Control Center

Brightness/volume toggles; sleep action; publish to HA where applicable.

Debug & Perf

Hidden overlay; verify FPS/allocations.

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

Context: You are working in baba-dev/M5Tab5-UserDemo. Read docs/wireframes.md and docs/AI_Codex_Guide.md. Build only what those files describe. Use LVGL on Tab5 at 1280×720 landscape. Respect tokens in ui_theme.h. Keep changes minimal and isolated: prefer editing ui_home.*, ui_rooms.*, ui_security.*, ui_control_center.*, ui_components.*, or integration/*. Preload RGB565 assets sized to display; no runtime scaling. Use optimistic UI with rollback on timeout. Add a short note on performance and video of the result.

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
