@page player_event_handler PlayerEventHandler

## Overview

`PlayerEventHandler` is responsible for applying **player-scoped game events**
to a single `Player` instance. These events modify the player’s board state,
zones, cards, counters, arrows, and associated UI and logging output.

Each `PlayerEventHandler` instance is bound **1:1 to a Player** and is invoked
exclusively by @ref GameEventHandler after basic routing and validation of
incoming server events.

This class represents the lowest-level authoritative application of game state
changes on the client.

---

## Scope and Authority

`PlayerEventHandler` operates under the following guarantees:

- All incoming events are **authoritative** and already validated by the server
- Events refer to valid players, zones, and card identifiers
- Ordering is guaranteed by the server and preserved by `GameEventHandler`

As a result, the handler does **not** perform rule validation or permission
checks. Its responsibility is to *apply* state, not *decide* legality.

---

## Responsibilities

### 1. Applying player-specific state changes

The primary responsibility of `PlayerEventHandler` is to mutate player-owned
state, including:

- Cards (`CardItem`)
- Zones (`CardZoneLogic`)
- Counters (card-level and player-level)
- Attachments and arrows
- Zone configuration flags (reveal / peek behavior)

Many handlers update both **logical state** and **visual/UI state** in tandem.

---

### 2. Coordinating complex card movement

Some events—most notably card movement—require coordinated updates across
multiple systems. For example, `eventMoveCard()`:

- Removes the card from the source zone
- Updates ownership, visibility, and identity
- Handles attachments and arrow cleanup
- Emits undo-draw or move logs
- Inserts the card into the destination zone
- Updates menus, hover state, and graphics

This makes `PlayerEventHandler` intentionally stateful and tightly coupled to
the board implementation.

---

### 3. Emitting logging signals

Rather than writing directly to logs or widgets, `PlayerEventHandler` emits
structured Qt signals describing *what happened*, including:

- Chat messages
- Card movement
- Shuffles and randomization
- Reveals and peeks
- Counter and attribute changes

These signals are consumed by logging systems and UI components, keeping
presentation concerns out of the handler.

Logging signals may be emitted **before or after mutation**, depending on
whether later changes would invalidate log data (e.g. card identity).

---

### 4. Handling cross-player interactions

Although bound to a single player, some events necessarily affect other
players’ state, including:

- Moving cards between players
- Attaching cards to another player’s permanents
- Creating arrows targeting other players or cards

In these cases, `PlayerEventHandler` performs the minimal required mutation
while respecting ownership boundaries enforced elsewhere.

---

## Event Dispatch Model

`PlayerEventHandler` exposes a single public entry point:

- `processGameEvent()`

This method:

1. Receives a generic `GameEvent`
2. Switches on `GameEventType`
3. Extracts the appropriate protobuf extension
4. Forwards the event to a typed handler

This keeps the event routing centralized and makes it easy to audit coverage
when new game events are introduced.

Unhandled events are logged as warnings.

---

## Event Categories

For clarity, event handlers are grouped conceptually into:

- **Chat and randomization**
    - Chat messages
    - Shuffles
    - Dice rolls

- **Arrows and targeting**
    - Create / delete arrows

- **Card and token creation**
    - Token generation
    - Counter creation

- **Card attributes and counters**
    - Tapped state
    - Power/toughness
    - Annotations
    - Card- and player-level counters

- **Zone-level operations**
    - Card movement
    - Zone dumps
    - Card destruction
    - Attachments

- **Draw and reveal**
    - Drawing cards
    - Reveals, peeks, and reveal windows

- **Zone configuration**
    - Always-reveal and always-look-at-top-card flags

This grouping mirrors the structure of the header file and reflects the
conceptual responsibilities of the class.

---

## Relationship to GameEventHandler

`PlayerEventHandler` is never instantiated or invoked directly by UI code.
Instead:

1. `GameEventHandler` receives a `GameEventContainer`
2. Player-scoped events are routed to the appropriate `PlayerEventHandler`
3. Global or spectator events are handled elsewhere

This separation keeps game-wide logic decoupled from player board state and
makes reconnection and replay handling simpler.

---

## Design Intent

`PlayerEventHandler` is designed to be:

- **Authoritative** – applies server state exactly as received
- **Stateful** – maintains consistency across cards, zones, and UI
- **Explicit** – one handler per event type
- **UI-aware** – updates views and menus as part of state mutation
- **Auditable** – easy to trace what code handles which event

---

## Related Classes

- @ref PlayerEventHandler
- @ref GameEventHandler
- `Player`
- `CardItem`
- `CardZoneLogic`
- `GameEvent`
- `GameEventContext`

---

## See Also

- @ref GameLogicPlayers
- @ref GameLogic
