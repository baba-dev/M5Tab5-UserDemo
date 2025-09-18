# Dev Setup

## Toolchain requirements

- Install and use ESP-IDF 5.4 or newer. The CI pipeline builds against the
  `release-v5.4` toolchain to ensure the project uses the NG I²C driver stack
  exclusively.
- Run `python3 fetch_repos.py` before building so that managed components are
  pulled into the workspace or refreshed to the pinned revisions.

## Build checklist

1. Set the target with `idf.py set-target esp32p4` the first time you configure a
   new working tree.
2. Run `idf.py build` from `platforms/tab5/` before sending a change for review.
3. Format any modified C or C++ sources with `idf.py clang-format` (or invoke
   `clang-format` manually).
4. If you touch component metadata, run `idf.py lint` to validate the manifest
   files.
