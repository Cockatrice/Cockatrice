# Limbo, Refuge and Oblivion piles

Continuumatrice adds **three persistent public card piles per player** alongside the existing Deck, Hand, Graveyard, Exile, Sideboard, Table and Stack zones.

| Display name | Stable wire-zone name | Visibility | Interactions |
| --- | --- | --- | --- |
| Limbo | `limbo` | Public | Drop cards, move selected cards, view contents |
| Refuge | `refuge` | Public | Drop cards, move selected cards, view contents |
| Oblivion | `oblivion` | Public | Drop cards, move selected cards, view contents |

These use existing `PileZoneLogic`, `PileZone`, server `Server_CardZone` and string-based protocol zone names. **The piles do not enforce gameplay rules** (e.g., what gets sent to Limbo or whether Oblivion is permanent). Nothing automatically sends cards to them; users move cards through drag/drop or the card **Move to** menu.

The three new piles appear in a second, compact column, aligned with the existing deck, graveyard and exile piles. The table and hand shift to allow the extra column without overlap. Double-click a new pile to open the standard zone viewer; right-click for a **View** menu. The card count and the pile name/icon are shown even when the pile is nonempty.

## Icons and asset overrides

The fork bundles original fallback SVG icons:

- `cockatrice/resources/piles/limbo.svg`
- `cockatrice/resources/piles/refuge.svg`
- `cockatrice/resources/piles/oblivion.svg`

They are registered in `cockatrice/cockatrice.qrc`; you do not need to manually copy them to run the built client.

To assign your own art **without recompiling**, put a matching asset in the **currently selected Continuumatrice theme**:

```text
~/.local/share/Continuumatrice/themes/<YourTheme>/piles/
  limbo.svg
  refuge.svg
  oblivion.svg
```

The theme can use `.png`, `.jpg`, `.jpeg` or `.svg`, and the existing theme manager also supports scheme-specific file variants such as `limbo-dark.svg`. The theme's asset takes precedence; the bundled SVG is a fallback. Reload/change the theme to refresh the icons. The icons are cosmetic; all card data remains in the database and the runtime zone state.

## Multiplayer compatibility

The new piles are created in **both** client-side `PlayerLogic` and server-side `Server_Player::setupZones`. They therefore work with the fork's **local game** and a **Continuumatrice-built server**. An unmodified upstream Cockatrice server does not create these zones and may reject card moves to `limbo`, `refuge` or `oblivion`. Use a matched client/server build for multiplayer.

No new protobuf messages are needed; the existing `Command_MoveCard` uses zone-name strings. Older upstream clients do not have dedicated icons/menus for these piles.

## Manual verification (after rebuild)

1. Launch a local game with two players and confirm all three new piles appear on each side.
2. Confirm the new icons and labels are distinct and the board does not overlap the old piles, stack, hand or table.
3. Move one card from the hand/table to each pile using **Move to**, and then by dragging.
4. Double-click or right-click each pile to view its card list, then move a card back to the table or hand.
5. Check the pile counters and name badges are legible when cards are present.
6. Rejoin/reconnect a game (and try a replay) to ensure pile contents are reconstructed from server zones.
7. Test both mirrored player orientations and the horizontal-hand setting at different window sizes.
8. Override an icon with an asset in the selected theme and reload the theme.
9. If using a remote server, ensure that server also has this fork's zone definitions.

Build locally with `cmake --build build --target cockatrice --parallel 16`. This change has **not** been compiled or visually run in this environment; the compilation and runtime checks above are essential before merging.
