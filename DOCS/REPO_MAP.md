# Repository map

- `platforms/tab5/` – ESP-IDF application for the Tab5 hardware. Contains the
  board-specific `main/` component, sdkconfig defaults, partition table, and
  audio assets that ship with the firmware image.
- `custom/` – application-specific UI, integration layers, and runtime asset
  loaders. Files here are shared across hardware and desktop targets.
- `custom/app_trace.h` – shared logging helpers available to both firmware and
  host targets; the former `components/core/` directory has been removed.
- `tests/` – host-side unit tests, LVGL snapshot tests, and future firmware
  smoke tests. Invoked with `make test`.
- `tools/` – development utilities such as format/lint wrappers and the asset
  conversion pipeline.
- `DOCS/` – developer-facing documentation including this map and flash layout
  notes.
- `ai/tasks/` – repeatable briefs for automated agents. Copy the template and
  fill in the context, goal, and acceptance criteria when scoping new work.
