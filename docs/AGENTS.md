# Documentation Rules

These instructions apply to files under `docs/`.

- Keep Markdown headings consistent with the existing documents (title case for H1, sentence case for deeper levels).
- Wrap lines at ≤ 100 characters where practical to keep diffs reviewable.
- Use relative links (`[text](./other_doc.md)`) so the files render correctly both on GitHub and inside firmware tooling.
- When you adjust wireframes or design tokens, call out the change in the PR description so UI contributors can sync their work.
- Include updated screenshots/mockups if the document describes UI that changed.
- After editing Markdown, run `npx markdownlint docs` (or another Markdown linter) and fix reported issues before committing.
