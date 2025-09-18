# Contributing guide

Thanks for helping improve the Tab5 firmware! This document outlines the
minimum steps required to get a change merged.

## Prerequisites

1. Install the ESP-IDF toolchain (v5.5) or use the provided devcontainer.
2. Install Python dependencies:
   ```bash
   make setup
   ```
3. Configure Git LFS so binary blobs (fonts, PNGs) stay lightweight:
   ```bash
   git lfs install
   ```

## Workflow

1. Create a feature branch and make focused commits. Follow Conventional Commit
   prefixes (e.g. `feat(core): add foo`).
2. Run the full validation suite before pushing:
   ```bash
   make fmt
   make build
   make test
   ./tools/check_secrets.sh
   ```
3. If you touch firmware components or enable new compiler warnings, also run:
   ```bash
   make tidy
   ```
4. Document user-visible behaviour in `CHANGELOG.md` and update any relevant
   docs in `docs/` or `DOCS/`.
5. For UI tweaks provide screenshots or LVGL snapshot updates.

## Pull requests

- Fill out the PR template, ticking the boxes you exercised locally.
- Reference issues in the footer (`Refs #123`).
- Keep diffs reviewable (prefer < 500 lines changed).
- Ensure CI workflows (`IDF Build`, `Host Tests`, `CodeQL`) are green before
  requesting review.
