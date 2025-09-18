# Repository map

- `platforms/tab5/` – ESP-IDF application for the Tab5 hardware. Contains the
  board-specific `main/` component, sdkconfig defaults, partition table, and
  audio assets that ship with the firmware image.
- `components/core/` – device-agnostic logic (ring buffer, settings store,
  future shared services) that is built both for firmware and host tests.
- `custom/` – application-specific UI, integration layers, and runtime asset
  loaders. Files here are shared across hardware and desktop targets.
- `tests/` – host-side unit tests, LVGL snapshot tests, and future firmware
  smoke tests. Invoked with `make test`.
- `tools/` – development utilities such as format/lint wrappers and the asset
  conversion pipeline.
- `DOCS/` – developer-facing documentation including this map and flash layout
  notes.
- `ai/tasks/` – repeatable briefs for automated agents. Copy the template and
  fill in the context, goal, and acceptance criteria when scoping new work.
