# Agent Contribution Contract

This repository ships firmware for the M5Stack Tab5 using ESP-IDF and LVGL.  
Follow the rules below whenever you make changes in this repo.

## Commit & PR discipline
- **Commit message format:** Use Conventional Commits: `type(scope): summary`.
  - Allowed types: `feat`, `fix`, `refactor`, `perf`, `docs`, `test`, `build`, `ci`, `chore`.
  - Keep the summary ≤ 72 characters, use imperative mood, and wrap body text at ≤ 72 characters per line.
  - Reference issues in the footer (e.g., `Refs #123`).
- **Small, reviewable PRs:**
  - Keep each PR focused on a single feature or bugfix.
  - Prefer ≤ ~500 lines of diff (adds + removals) per PR.
  - Update docs/assets alongside code so reviewers see a complete change.
  - Provide context in the PR description: what changed, why, and how it was verified (include screenshots/GIFs for UI changes).

## Required checks before requesting review
Run these from the repository root unless noted otherwise.

| Command | When to run | Notes |
| --- | --- | --- |
| `idf.py set-target esp32p4` | First build in a fresh checkout | Only needed once per workspace unless you clean the build folder.
| `idf.py build` | Every PR | Must succeed; catches most integration issues.
| `bash scripts/preflight.sh` | Before requesting review | Runs repo installer, fetches deps, builds Tab5, runs tests, and checks for boot loops when SERIAL_PORT is set. |
| `idf.py clang-format` <br>or `clang-format -style=file -i <files>` | Whenever C/C++ is touched | Uses the repo's `.clang-format` (clang-format 13). Stage formatted files.
| `idf.py lint` | When modifying components or build metadata | Ensures `idf_component.yml` metadata stays valid; safe to run even if no changes were made.
| `ctags -R` / IDE index refresh | Optional but recommended | Keeps symbol navigation accurate after large refactors.

If you add or modify scripts/tests in other languages, run the relevant linters (e.g., `python -m compileall` for Python, `npx markdownlint docs` for Markdown) and mention them in the PR.

## Coding style & structure
- Respect the project `.clang-format` for all C/C++/Obj-C files.
- Match existing naming in the file you touch. Defaults:
  - Classes/structs/interfaces: `PascalCase`.
  - Member functions: `PascalCase`.
  - Free/static functions: `snake_case` (mirrors existing helpers like `on_startup_anim`).
  - Constants: `kPascalCase` for globals, `SCREAMING_SNAKE_CASE` for macros.
- Keep includes grouped and ordered: standard library, third-party, then project headers. Let `clang-format` finalize spacing.
- New firmware code lives under `custom/` (UI, integration, platform) unless you are patching upstream vendor code in `app/`.
- Add SPDX headers to new source files to match the rest of the codebase.
- Do not commit generated artifacts (build outputs, `.vscode`, etc.).

---

The instructions above apply to the entire repository unless a more specific `AGENTS.md` exists deeper in the tree.
