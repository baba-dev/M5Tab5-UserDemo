# Asset Pipeline

Use this checklist when creating or updating visual and audio assets so the
firmware keeps a small footprint and renders crisply on the Tab5 display.

## File organization

- Store production-ready assets in `custom/assets/`. Keep source design files
  (Figma, PSD) out of the repository; attach them to design tickets instead.
- Group assets by usage: `icons/`, `images/`, `backgrounds/`, `fonts/`, and add
  screen-specific subfolders when helpful.
- Document new assets in the table below so AI agents know which files already
  exist.

| File | Purpose | Notes |
| --- | --- | --- |
| _TBD_ | Populate as assets are created. | Include resolution, depth, owner. |

## Export settings

- Color format: **RGB565 little-endian** (LVGL friendly) with no alpha channel.
- Dimensions: match the final on-screen size; avoid runtime scaling to preserve
  performance.
- Naming: lowercase with dashes, e.g., `icon-lights-rgb565.bin`. Include the
  color depth suffix when ambiguous.
- Optimize assets using `lvgl_img_conv.py` or `ImageMagick` scripts before
  committing.

## Registration in code

- Reference new binaries in the relevant `CMakeLists.txt` within `custom/` so
  they are flashed with the firmware.
- Register LVGL image descriptors in a central header (e.g.,
  `custom/assets/assets.h`) to keep lookups consistent. Add forward declarations
  instead of duplicating descriptors across screens.
- Update `README.md` or per-screen documentation when a new asset changes the
  visual spec.

## Accessibility and localization

- Prefer icons with clear silhouettes that remain legible at 56×56 px touch
  targets.
- Provide alternative text descriptions in UI code to support future
  text-to-speech or analytics.
- When adding text baked into images, create localized variants or avoid
  rasterized text entirely.

## Quality assurance checklist

- [ ] Asset file passes LVGL image converter validation with no warnings.
- [ ] Asset referenced in code builds successfully on hardware.
- [ ] Visual inspection on device confirms sharp edges and accurate colors.
- [ ] Documentation updated (this file and any affected screen specs).
