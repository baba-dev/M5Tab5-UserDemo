# Repo map

A generated overview of the repository structure with short descriptions for quick orientation.

```text
.
├── app/                     - Upstream Tab5 application framework and Mooncake apps
│   ├── apps/                - Built-in demo apps bundled with the vendor firmware
│   ├── assets/              - Stock images, fonts, and other binary resources from M5Stack
│   ├── hal/                 - Hardware abstraction layer provided by the upstream demo
│   ├── shared/              - Common utilities reused across vendor apps
│   ├── app.cpp, app.h       - Entry points that boot the Mooncake runtime
│   └── CMakeLists.txt       - Component definition for the upstream app bundle
├── custom/                  - All Tab5-specific extensions maintained in this repo
│   ├── assets/              - Custom RGB565 graphics and fonts for the new UI
│   ├── integration/         - MQTT, Home Assistant, and Frigate glue code
│   ├── platform/            - Drivers and platform helpers layered on the HAL
│   ├── ui/                  - LVGL screens, widgets, and layout logic
│   └── AGENTS.md            - Contribution rules for the custom modules
├── docs/                    - Architecture notes, prompts, and contributor guidance
│   ├── ARCHITECTURE.md      - High level system overview
│   ├── AI_GUIDELINES.md     - Expectations for AI and automation contributors
│   ├── AI_Codex_Guide.md    - Detailed AI workflow and escalation plan
│   ├── ASSETS.md            - Asset pipeline and quality checklist
│   ├── PERFORMANCE.md       - Profiling notes and performance budgets
│   ├── TASKS.md             - Roadmap and milestone tracker
│   ├── mqtt_contract.md     - MQTT topic map and payload schemas
│   ├── REPO_MAP.md          - This repository map
│   ├── entities.json        - Home Assistant entities consumed by the UI
│   ├── prompts/             - Task-specific prompt snippets for agents
│   └── wireframes.md        - UI wireframes for the Tab5 experience
├── platforms/               - Board-specific configuration from the vendor demo
├── .devcontainer/           - VS Code / Codespaces environment definition
├── .github/                 - GitHub Actions workflows and repository templates
├── fetch_repos.py           - Helper script to pull LVGL and component submodules
├── repos.json               - Manifest describing the extra repositories to fetch
├── CMakeLists.txt           - Top level build definition for ESP-IDF
├── lv_conf.h                - LVGL configuration used by the project
└── README.md                - Quick start guide and contributor overview
```

For deeper detail on MQTT topics and Home Assistant entities, see
[`docs/mqtt_contract.md`](./mqtt_contract.md) and [`docs/entities.json`](./entities.json).

## Rooms page layout

The Rooms experience lives in `custom/ui/pages/ui_page_rooms.*` with its card widget helpers
under `custom/ui/widgets/`. It renders a fixed three-column grid sized for the Tab5's 1280×720
landscape canvas. Each column hosts a glassmorphic room card (Bakery, Bedroom, Living Room)
with a master toggle, temperature and humidity readout, and animated entry/interaction states.
