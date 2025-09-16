# AI Execution Guidelines

Read this checklist before using an AI coding assistant on the project. It
complements `docs/AI_Codex_Guide.md` by describing the workflow expectations,
safety rails, and formatting rules for automated contributions.

## Orientation steps

1. Re-read `docs/wireframes.md` and confirm the screen or component you plan to
   modify.
2. Scan `docs/TASKS.md` to pick the highest-priority open task that matches your
   scope.
3. Inspect source files to understand naming conventions, namespaces, and
   helper patterns.
4. Record open questions in the pull request description or inline TODOs so a
   human reviewer can resolve them.

## Implementation playbook

- Modify only the files required for the chosen task; avoid shotgun refactors.
- Prefer extending `custom/` modules over editing vendor code in `app/` unless
  the task explicitly requires it.
- Maintain optimistic UI interactions with clear rollback paths when publishing
  MQTT commands.
- Reuse theme tokens from `ui_theme.h` and shared components from `custom/ui/`
  to keep screens consistent.
- When adding new assets or configuration values, update the relevant
  documentation (`docs/ASSETS.md`, `docs/ARCHITECTURE.md`, or `README.md`).

## Testing expectations

- Run `idf.py build` before finalizing a change. Resolve compiler warnings
  rather than silencing them.
- Execute `idf.py lint` when modifying integration metadata or component
  manifests.
- Run `npx markdownlint docs` after editing Markdown. Fix or justify any
  violations in the PR notes.
- Capture on-device footage or simulator screenshots for UI-heavy changes and
  attach them to the PR.

## Commit hygiene

- Use Conventional Commit messages as described in the repository `AGENTS.md`.
- Keep commits focused. If the AI generates a broad refactor, split the work
  manually before submitting a PR.
- Document limitations or follow-up ideas in the PR description under a "Next
  steps" heading.

## Escalation rules

- If a task conflicts with performance limits or design tokens, update the docs
  first, then land the code change in a follow-up PR.
- When encountering unclear MQTT payloads or stream behaviors, extend the
  contract in `docs/mqtt_contract.md` and flag the ambiguity in the roadmap.
- Avoid introducing new third-party dependencies without explicit human
  approval.
