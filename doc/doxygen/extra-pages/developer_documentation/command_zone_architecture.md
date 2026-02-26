@page command_zone_architecture Command Zone Architecture

The command zone feature implements Commander format support in Cockatrice,
providing zones for commander cards with manual tax tracking.

# Architecture Overview

The command zone follows Cockatrice's logic/graphics separation pattern:

```
┌───────────────────────────────────────────────────┐
│ Graphics: CommandZone, ZoneToggleButton,          │
│           CommanderTaxCounter                     │
└───────────────────────────────────────────────────┘
                    │ signals/slots
                    ▼
┌───────────────────────────────────────────────────┐
│ Logic: CommandZoneLogic (card storage, positions) │
└───────────────────────────────────────────────────┘
```

# Zone Types

| Zone | Name Constant | Purpose | Enablement |
|------|---------------|---------|------------|
| Primary | `ZoneNames::COMMAND` | Main commander, always visible | `enable_command_zone` |
| Partner | `ZoneNames::PARTNER` | Partner commander, collapsible | `enable_command_zone` |
| Companion | `ZoneNames::COMPANION` | Ikoria companion card | `enable_companion_zone` |
| Background | `ZoneNames::BACKGROUND` | Baldur's Gate background | `enable_background_zone` |

All zones use the same `CommandZone` and `CommandZoneLogic` classes,
differentiated by the `isPrimaryZone` flag. Primary and Partner zones are always
created together when command zone is enabled. Companion and Background zones
can be enabled independently via separate game creation options.

Primary and Partner zones hold a single card (commander rules, tax counter positioning); Companion and Background support multiple cards for edge cases. See `supportsMultipleCards()` in `command_zone_types.h`.

# Key Components

## CommandZoneLogic
Manages the card list for a command zone. Overrides `addCardImpl()` to support
positional insertion when cards are reordered via drag-and-drop.

@see CommandZoneLogic

## CommandZone
Graphics layer handling rendering and input. Features:
- Vertical card centering
- Expand/collapse for partner zone
- Minimize mode (25% height)
- Drag-and-drop support

@see CommandZone

## CommanderTaxCounter
Visual counter showing cumulative commander tax. Positioned at top-left of
the zone. Players manually increment the counter when casting their commander.

@see CommanderTaxCounter

## ZoneToggleButton
Small button at the seam between zones. Clicking toggles the sibling zone
visibility. Used by the primary command zone to toggle the partner zone, and
can be reused by any zone that needs to toggle a sibling.

@see ZoneToggleButton

# Common Modifications

## Adding a new zone property
1. Add the property to `CommandZoneLogic`
2. Emit a signal when the property changes
3. Connect the signal in `CommandZone` to update visuals

## Changing zone appearance
1. Modify `CommandZone::paint()` for background/border
2. Modify `reorganizeCards()` for card layout
3. Update `boundingRect()` if size changes

## Modifying tax behavior
The tax counter value is managed by the server. To change tax logic:
1. Server-side changes in `libcockatrice_network/libcockatrice/network/server/remote/game/server_player.cpp`
2. Counter display changes in `CommanderTaxCounter`

# Menu Integration

The command zone context menus are managed by `PlayerMenu`:

| Menu | Zone | In Player Menu | Special Features |
|------|------|----------------|------------------|
| commandZoneMenu | Primary | Yes | View partner zone, partner toggle, tax counters for both |
| partnerZoneMenu | Partner | No (zone right-click only) | Self-toggle, partner tax counter |
| companionZoneMenu | Companion | Yes | Standard zone actions, no tax counter |
| backgroundZoneMenu | Background | Yes | Standard zone actions, no tax counter |

The `commandZoneMenu` consolidates all commander-related actions including partner
zone access ("View partner zone" action). The `partnerZoneMenu` is not added to
the main player menu but remains attached to the partner zone itself for direct
right-click access. This consolidation reduces menu clutter while maintaining
full functionality.

All menus are created in the `PlayerMenu` constructor and attached to their zones
in `setMenusForGraphicItems()`. Companion and Background zones have simpler
menus without tax counter adjustments (these mechanics don't use commander tax).

@see CommandZoneMenu
@see PlayerMenu
