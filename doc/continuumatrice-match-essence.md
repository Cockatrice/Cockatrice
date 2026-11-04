# Match Essence pools (Continuum Universes CCG)

Continuumatrice matches provide **16 independent Essence balances per player**, using the canonical sequence:

```text
M S O G E A Q C J B R L V T P N
```

**J** is Jasper; **N** is colorless. These are not Magic mana colors, and Essence is not primarily obtained by drawing land cards.

## Behavior

The client displays a compact **4 × 4 Essence Pool** in a reserved column beside each player's battlefield. The panel shows the user's installed `icons/essence/<code>.svg` artwork and the current quantity, including zero.

- **Left-click:** add one Essence of that type (gain).
- **Right-click:** remove one Essence of that type (spend), when the balance is positive.
- **Shift + left-click:** set an exact nonnegative balance through a number dialog.
- Other players and spectators can **view** the balances; only the player controlling the pool (or a judge) can modify it.

For example, generating three Jasper Essence from a card ability currently means clicking `J` three times or setting its balance. Likewise, paying two Jasper means decrementing its balance twice. Future automated card effects should call a validated resource transaction action rather than simulating mouse clicks.

### What this change does **not** assume

It does not invent Essence generation rules, a land-based resource curve, turn-based Essence resets, automatic cost deduction, overdraft/Essence debt, or activation triggers. The core match tracks available resources, but **gameplay effects are still manual**. Define those rules separately before implementing them.

## Server-authoritative state

The existing Cockatrice-derived counter protocol already supports all the required functionality. The fork's shared `EssenceCounter` helper (`libcockatrice_utility/libcockatrice/utility/essence_counter.h`) defines stable counter names and the canonical symbol order.

The server creates sixteen counters at game setup with IDs `1..16` and names `essence:M`, `essence:S`, ..., `essence:N`. The existing life counter retains ID `0`. These names are protocol/storage identifiers; client gameplay displays the corresponding Essence symbols rather than exposing raw IDs.

Resource counters:
- Start at `0`, independently for every player.
- Cannot go below `0` (validated by the server for increment **and** set commands).
- Cannot be deleted or created a second time via the generic counter commands.
- Are serialized to client snapshots, replay events, and reconnect state by the existing counter protocol.
- Do not live on card ScriptableObjects, card metadata/XML, or a global static client singleton.

`Server_Player` implements this for both the self-hosted **Continuum-Servatrice** and the embedded/local game server because they share the same player server code. **Both client and server must use a matching Continuumatrice build**. The regular MTG Cockatrice client/server do not provide the native Essence Pool display.

## Implementation map

| File | Responsibility |
| --- | --- |
| `libcockatrice_utility/libcockatrice/utility/essence_counter.h` | Canonical Essence order, protocol names, nonnegative values |
| `libcockatrice_card/libcockatrice/card/essence_types.h` | Card-cost parser uses the same source of symbols |
| `libcockatrice_network/libcockatrice/network/server/remote/game/server_player.cpp` | Server counter initialization, updates, permanent-counter restrictions |
| `cockatrice/src/game_graphics/player/essence_pool_item.h/.cpp` | Compact graphical resource display, manual controls, server-state observation |
| `cockatrice/src/game_graphics/player/player_graphics_item.cpp` | Places Essence pool beside player's board and excludes it from the narrow ordinary-counter stack |
| `tests/essence_types_test.cpp` | Protocol-name round trips and nonnegative value utility checks |

The panel never owns the true resource quantities. It listens for `CounterState::valueChanged` and sends normal `actIncrementCounter`/`actSetCounter` commands through `PlayerActions`. Only the server authoritatively changes the stored balance, so remote clients and replays see the same values.

## Build and tests

On the `feature/continuumatrice-essence-match-pools` branch:

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DWITH_CLIENT=ON -DWITH_SERVER=ON -DWITH_ORACLE=OFF -DTEST=ON

cmake --build build --target continuumatrice continuum-servatrice essence_types_test --parallel 16

ctest --test-dir build -R EssenceTypesTest --output-on-failure

./build/continuumatrice/continuumatrice
```

## Functional acceptance checklist

- [ ] In a new local game, each player has a 4×4 pool with exactly 16 Essence types and zero balances.
- [ ] Gaining `J` leaves `C`, `B`, and `N` unchanged; right-clicking `J` at zero cannot make it negative.
- [ ] Shift+left can set `N` (colorless) to a specific nonnegative amount.
- [ ] The normal life counter still works; no default MTG W/U/B/R/G/X/Storm counters appear.
- [ ] Both players see the same balance after a command is processed by the server.
- [ ] A spectator can view but not modify another player's Essence.
- [ ] Disconnect/reconnect restores all 16 values from the game-state snapshot.
- [ ] Replay playback restores previously recorded Essence changes.
- [ ] A remote game running Continuum-Servatrice matches the local game behavior.
- [ ] The Essence panel does not overlap the three battlefield bands or Limbo/Refuge/Oblivion piles at normal zoom.

**Current status:** Source implementation committed. This server/client change has **not** been compiled or tested in a live match yet. The new browser and match UI need user-side smoke testing.
