# Flash layout

The Tab5 firmware uses the partition table at `platforms/tab5/partitions.csv`:

| Partition | Type | Offset  | Size | Notes |
| --------- | ---- | ------- | ---- | ----- |
| `nvs`     | data | 0x9000  | 24KB | Non-volatile key/value storage. |
| `phy_init`| data | 0xF000  | 4KB  | Wi-Fi PHY calibration data. |
| `factory` | app  | 0x10000 | 10MB | Main application image. |
| `human_face_det` | data | auto | 400KB | SPIFFS partition reserved for demo assets. |
| `storage` | data | auto | 2MB | SPIFFS partition that carries generated assets (`custom/assets` + `out/assets`). |

`make assets` produces the runtime blobs in `out/assets/`; the ESP-IDF build
copies them into the `storage` partition via the staging step in
`platforms/tab5/main/CMakeLists.txt`.
