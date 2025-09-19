# Tracepoints

Use the macros in `custom/app_trace.h` to annotate important
runtime events. Common sections/events:

| Section  | Event          | Purpose                                  |
| -------- | -------------- | ---------------------------------------- |
| `boot`   | `init`         | Firmware entry, board bring-up sequence. |
| `ui`     | `page_show`    | User navigates to a page.                |
| `ui`     | `asset_fallback` | File-system asset missing, using fallback. |
| `audio`  | `play`         | Playback triggered (song, chime).        |

Add new entries when introducing features so logs remain searchable.
