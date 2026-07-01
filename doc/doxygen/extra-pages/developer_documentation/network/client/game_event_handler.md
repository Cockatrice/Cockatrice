@page game_event_handler GameEventHandler

## Overview

`GameEventHandler` is the central coordinator for **game-wide commands and events**.
It acts as the bridge between the networking layer (protobuf messages received from
the server) and the local game model and UI.

Unlike `PlayerEventHandler`, which is responsible for player-scoped state and zones,
`GameEventHandler` handles:

- Global game flow (turns, phases, host changes)
- Player and spectator lifecycle events
- Chat and logging events
- Dispatching incoming events to the appropriate subsystem
- Sending locally initiated commands to the server

In short, it is the **top-level event dispatcher** for an active game.

---

## Responsibilities

`GameEventHandler` has four primary responsibilities:

### 1. Sending game-wide commands

UI actions that affect the game as a whole (e.g. advancing the turn, changing phases,
sending chat messages) are translated into protobuf commands and sent to the server
via this class.

Examples include:

- Advancing or reversing the turn order
- Conceding or unconceding
- Changing the active phase
- Leaving the game
- Sending chat messages

These commands are wrapped in `PendingCommand` instances so their lifecycle and
responses can be tracked.

---

### 2. Processing incoming game events

Incoming server messages arrive as a `GameEventContainer`.
`GameEventHandler` is responsible for:

- Emitting lifecycle signals before and after processing
- Dispatching each event to the correct handler
- Forwarding player-scoped events to `PlayerEventHandler` instances
- Handling spectator-only and game-global events directly

This design keeps networking concerns isolated from game logic and UI code.

---

### 3. Managing global game state transitions

Certain events affect the entire game state and require coordinated updates across
multiple subsystems. Examples include:

- Full game state synchronization
- Active player or phase changes
- Game host changes
- Game closure
- Turn reversal

`GameEventHandler` ensures these events are processed in a consistent order and
that all interested systems are notified via Qt signals.

---

### 4. Emitting UI and logging signals

Rather than directly manipulating UI widgets, `GameEventHandler` emits
high-level signals that are consumed by:

- Game widgets
- Player and spectator lists
- Message and event logs
- Status indicators (ready state, deck selection, connection state)

This keeps the handler independent of concrete UI implementations.

---

## Relationship to PlayerEventHandler

`GameEventHandler` and `PlayerEventHandler` work together but have distinct roles:

| GameEventHandler | PlayerEventHandler |
|------------------|--------------------|
| Global game state | Per-player state |
| Turn / phase flow | Zones and cards |
| Player join/leave | Player actions |
| Spectator events | Player-specific events |
| Chat dispatch | Card and zone updates |

When a server event is associated with a specific player, `GameEventHandler`
routes it to the corresponding `PlayerEventHandler`. Events without a player
context are handled directly.

---

## Event Processing Flow

A typical incoming event flow looks like this:

1. `AbstractClient` receives a `GameEventContainer`
2. `GameEventHandler::processGameEventContainer()` is called
3. `containerProcessingStarted()` is emitted
4. Each event is:
    - Handled directly **or**
    - Forwarded to a `PlayerEventHandler`
5. Logging and UI signals are emitted as needed
6. `containerProcessingDone()` is emitted

This structured flow makes it easy to:

- Suppress UI updates during replays
- Handle reconnections cleanly
- Detect flood protection or error states

---

## Command Lifecycle

Outgoing commands follow a similar structured path:

1. UI action triggers a `handle*()` method
2. A protobuf command is constructed
3. The command is wrapped in a `PendingCommand`
4. `sendGameCommand()` sends it to the server
5. `commandFinished()` receives the response
6. Errors or flood conditions are handled centrally

This approach avoids duplicated error handling across UI code.

---

## Design Goals

`GameEventHandler` is designed to be:

- **Centralized** – one entry point for all game-wide events
- **UI-agnostic** – communicates via signals, not widgets
- **Predictable** – well-defined event ordering and lifecycle
- **Composable** – works in concert with `PlayerEventHandler`
- **Testable** – logic is separated from rendering

---

## Related Classes

- @ref GameEventHandler
- @ref PlayerEventHandler
- `GameEventContainer`
- `PendingCommand`
- `AbstractClient`
- `AbstractGame`

---

## See Also

- @ref GameLogic
- @ref PlayerEventHandler
- @ref GameState
