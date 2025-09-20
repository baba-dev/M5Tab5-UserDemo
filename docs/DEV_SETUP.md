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

### Wi-Fi remote firmware

ESP-IDF 5.4.2 ships the ESP32 Wi-Fi remote project in
`$IDF_PATH/components/espressif__esp_hosted/slave`. Build and flash it once so
the Tab5 SDIO link negotiates correctly:

```bash
cd "$IDF_PATH/components/espressif__esp_hosted/slave"
idf.py set-target esp32
idf.py build
idf.py -p <remote_port> flash
```

The build drops `build/network_adapter.bin` if you prefer flashing with
`esptool.py`. Future ESP32-C6 variants should publish their binaries as external
artifacts instead of committing them to the repo.
