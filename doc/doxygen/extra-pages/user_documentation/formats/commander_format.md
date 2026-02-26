@page commander_format Commander Format Support

# Overview

Cockatrice provides dedicated support for the Commander (EDH) format, including specialized zones for managing your commander and partner commander cards.

## What is the Command Zone?

The Command Zone is a special area where your commander resides when not on the battlefield. In Cockatrice, this is represented as a dedicated zone that appears above the stack when playing Commander format games.

Key features:
- **Primary Command Zone**: Holds your main commander card
- **Partner Zone**: Expandable secondary zone for partner commanders
- **Companion Zone**: Optional zone for Ikoria companion cards
- **Background Zone**: Optional zone for Baldur's Gate background enchantments
- **Commander Tax Counters**: Visual tracking of commander tax (the additional mana cost each time you cast your commander from the command zone)

## Enabling the Command Zone

The Command Zone automatically appears when the game server indicates you're playing a Commander format game. If you don't see the Command Zone, check that:
1. The game room is configured for Commander format
2. The server supports command zones

## Using the Command Zone

### Designating a Commander

To place a card in the Command Zone:
1. Drag the card from your hand, library, or other zone to the Command Zone
2. Primary and Partner zones accept one card each; Companion and Background zones can hold multiple cards (for edge-case mechanics like Rule 0 multiple companions)

### Zone States

All zones toggle between **full size** and **minimized (25% height)** via double-click. Tax counters remain visible when minimized.

### Partner Zone

Click the toggle button at the bottom of the Command Zone (or use the context menu) to expand/collapse the Partner Zone. It behaves identically to the primary zone with its own card display, minimize capability, and dedicated Partner Tax counter.

> **Note**: The Partner Zone will not collapse if it contains a card or has a tax counter value greater than 0. This prevents accidentally hiding important game state.

### Companion Zone

The Companion Zone is designed for the Ikoria companion mechanic:

**When to use**: If your deck includes a companion card (a creature that can start outside the game if your deck meets its restriction), place it in the Companion Zone.

**Enabling**: The Companion Zone must be enabled when creating the game (see Game Creation Options below).

**Features**:
- Displays your companion card(s) publicly (supports multiple for Rule 0 edge cases)
- No tax counter (companion mechanic doesn't use commander tax)
- Collapsible to save screen space

### Background Zone

The Background Zone is designed for the Baldur's Gate "Choose a Background" mechanic:

**When to use**: If your commander has "Choose a Background" (like many commanders from Commander Legends: Battle for Baldur's Gate), place your background enchantment in this zone.

**Enabling**: The Background Zone must be enabled when creating the game (see Game Creation Options below).

**Features**:
- Displays your background enchantment(s) publicly (supports multiple for edge cases)
- No tax counter (backgrounds don't have commander tax)
- Collapsible to save screen space

## Commander Tax Tracking

Cockatrice tracks commander tax with dedicated counters:

- **Commander Tax Counter**: Appears in the top-left of the Command Zone
- **Partner Tax Counter**: Appears in the top-left of the Partner Zone (when expanded)

### Adjusting Tax Counters

Using the context menu (right-click on the zone):
- "Increase Commander Tax Counter (+1)" - Increments the tax by 1
- "Decrease Commander Tax Counter (-1)" - Decrements the tax by 1

Tax counters track the number of times your commander has been cast during the game.

## Context Menu Options

The battlefield's "Commander" submenu provides consolidated access to all zone actions: view zones, adjust tax counters (+1/-1), and toggle partner/companion/background zones. Right-clicking directly on a zone shows zone-specific options (view, tax counter for Primary/Partner, toggle, minimize).

## UI Summary

| Element | Location | Action |
|---------|----------|--------|
| Command Zone | Above the stack | Holds primary commander |
| Partner Zone | Below Command Zone (when expanded) | Holds partner commander |
| Companion Zone | Below Partner Zone (when enabled) | Holds companion card(s) |
| Background Zone | Below Companion Zone (when enabled) | Holds background enchantment(s) |
| Toggle Button | Bottom of Command Zone | Click to expand/collapse Partner Zone |
| Tax Counter | Top-left of Primary/Partner zones | Shows current commander tax |
| Minimize | Zone background | Double-click to toggle 25% height mode |

## Game Creation Options

When creating a game, you can configure which zones are available:

| Option | Default | Description |
|--------|---------|-------------|
| Enable Command Zone | Unchecked | Enables commander and partner zones with tax counters |
| Enable Companion Zone | Unchecked | Enables the Companion Zone for Ikoria companion mechanic |
| Enable Background Zone | Unchecked | Enables the Background Zone for Baldur's Gate backgrounds |

All three options are independent checkboxes. You can enable any combination:
- **Command Zone only**: Standard Commander/EDH gameplay
- **Companion Zone only**: Use companion cards in any format
- **Background Zone only**: Use backgrounds without commander zones
- **Any combination**: Mix and match as your game requires

**Defaults for new games**: All zone options default to unchecked. Your preferences are saved for future games when "Remember settings" is enabled.

## Tips

- Tax counters are persistent throughout the game and visible to all players
- Cards in the Command Zone are visible to all players (public information)

## See Also

- @ref creating_decks - How to create and edit decks
- @ref editing_decks - Detailed deck editing guide
