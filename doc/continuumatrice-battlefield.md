# Continuumatrice battlefield style

Cockatrice's three visual table rows are reused as **Front**, **Support**, and **Command**. This is purely a display change: the server still sees a single `table` zone.

| Card XML `<tablerow>` | Position |
| --- | --- |
| `0` | Front |
| `1` | Support |
| `2` | Command |

## Optional color customization

The default zone colors are **Front `#dc143c` (Crimson)**, **Support `#c3b091`**, and **Command `#0a0e3f`**.

To override the defaults, create `battlefield.ini` in your active **Continuumatrice** theme directory:

```ini
[Zones]
Front = #dc143c
Support = #c3b091
Command = #0a0e3f
```

Qt accepts `#RRGGBB` for opaque colors and `#AARRGGBB` to specify transparency. The built-in defaults above are opaque; a theme can set translucent overrides explicitly. An existing `battlefield.ini` still takes precedence over these defaults. Reload the theme to apply changes. The default layout places Front closest to the opposing player's battlefield and Command closest to each player's hand. As in Cockatrice, the **Invert vertical coordinate** setting reverses the row placement (and its labels) when enabled.

## Font

To render labels using your own **Continuumorphous Regular**, copy either `Continuumorphous-Regular.ttf` or `Continuumorphous-Regular.otf` into `fonts/` inside the current theme directory, or into a `fonts/` directory beside the Continuumatrice executable.

The font is also detected if already installed as "Continuumorphous Regular" or "Continuumorphous". If not found, a Qt default font is used. The font asset is not included in this change; no resource entry was added that would break builds when the file is missing.

## Compatibility

The existing drag/drop, stacking, card counters, playmats, and multiplayer protocols are preserved. The automatic `tablerow` placement is explicitly mapped `0 -> Front`, `1 -> Support`, `2 -> Command` (not Cockatrice's reversed Magic-oriented mapping). This does **not** enforce gameplay placement restrictions.
