# Custom assets

Source-of-truth media lives here before it is converted into runtime-friendly blobs.
Run `make assets` to populate `out/assets/` and refresh the manifest that the
firmware consumes at startup.

| id | Source file | Mounted path |
| --- | --- | --- |
| *(example)* `icon_play` | `icons/play.webp` | `/spiffs/custom/assets/icons/play.webp` |

Add or update rows when you introduce new images or fonts so UI code can reference the
correct mount paths. Keep the filenames stable; bots only touch generated outputs under
`out/assets/`.
