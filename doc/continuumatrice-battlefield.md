# Continuumatrice battlefield style

Cockatrice's three visual table rows are reused as **Front**, **Support**, and **Command**. This is purely a display change: the server still sees a single `table` zone.

| Card XML `<tablerow>` | Position |
| --- | --- |
| `0` | Front |
| `1` | Support |
| `2` | Command |

## Optional color customization

Create `battlefield.ini` in the active Cockatrice theme directory:

```ini
[Zones]
Front = #A08E343D
Support = #A0286981
Command = #A05E428C
```

The Qt color notation is `#AARRGGBB`, including transparency. Without this file, built-in red, teal, and purple shades are used. Reload the theme to apply changes. The default layout places Front closest to the opposing player's battlefield and Command closest to each player's hand. As in Cockatrice, the **Invert vertical coordinate** setting reverses the row placement (and its labels) when enabled.

## Font

To render labels using your own **Continuumorphous Regular**, copy either `Continuumorphous-Regular.ttf` or `Continuumorphous-Regular.otf` into `fonts/` inside the current theme directory, or into a `fonts/` directory beside the Continuumatrice executable.

The font is also detected if already installed as "Continuumorphous Regular" or "Continuumorphous". If not found, a Qt default font is used. The font asset is not included in this change; no resource entry was added that would break builds when the file is missing.

## Compatibility

The existing drag/drop, stacking, card counters, playmats, and multiplayer protocols are preserved. The automatic `tablerow` placement is explicitly mapped `0 -> Front`, `1 -> Support`, `2 -> Command` (not Cockatrice's reversed Magic-oriented mapping). This does **not** enforce gameplay placement restrictions.
