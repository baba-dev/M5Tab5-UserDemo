# Custom Module Rules

These rules apply to any files added under `custom/`.

## Module layout
- Mirror the existing structure when adding new features:
  - `custom/ui/` for LVGL views and widgets.
  - `custom/integration/` for Home Assistant, MQTT, or Frigate bindings.
  - `custom/platform/` for device drivers, HAL glue, or power management.
  - `custom/assets/` for RGB565 images, fonts, and other binary resources (pre-sized; no runtime scaling).
- Keep module boundaries clean: UI files should not contain MQTT logic; use thin integration helpers instead.

## Coding style
- Place public declarations in `*.h` and implementation in matching `*.cpp` unless the file is header-only for performance.
- Wrap new code in a descriptive namespace (e.g., `custom::ui`) to avoid collisions with upstream vendor code.
- Use `snake_case` for free helper functions and file-scope statics, `PascalCase` for classes/structs, and `kPascalCase` for constants.
- Include SPDX license headers at the top of every new file.
- Run `idf.py build` after adding assets to ensure they are referenced correctly in CMake component files.

## Assets
- Store assets in `RGB565` (little-endian) format at the exact resolution required by the UI.
- Update any asset manifest or registration tables when adding/removing files, and describe the change in the PR summary.
