@page protocol_game_command GameCommand (Protocol Concept)

# Game Commands Reference

This document describes game-level commands (`GameCommandType`) and how they are
handled across the server and client.

Flow overview:

Client
→ GameCommand
→ Server_Game::handle*
→ GameEvent(s)
→ PlayerEventHandler::event*

## Command Mapping

### `GAME_SAY` (1002)

**Purpose:** Send a chat message during a game.

**Server:**
- `Server_Game::handleGameSay`
- Emits `Event_GameSay`

**Client:**
- `PlayerEventHandler::eventGameSay`

---

### `SHUFFLE` (1003)

**Purpose:** Shuffle a card zone (usually library).

**Server:**
- `Server_Game::handleShuffle`
- Reorders cards in zone
- Emits `Event_Shuffle`

**Client:**
- `PlayerEventHandler::eventShuffle`
- Clears revealed top cards
- Closes affected zone views

---

### `ROLL_DIE` (1005)

**Purpose:** Roll one or more dice.

**Server:**
- `Server_Game::handleRollDie`
- Computes random values
- Emits `Event_RollDie`

**Client:**
- `PlayerEventHandler::eventRollDie`

---

### `DRAW_CARDS` (1006)

**Purpose:** Draw cards from deck.

**Server:**
- `Server_Game::handleDrawCards`
- Moves cards from deck → hand
- Emits `Event_DrawCards`

**Client:**
- `PlayerEventHandler::eventDrawCards`

---

### `UNDO_DRAW` (1007)

**Purpose:** Undo a previous draw.

**Server:**
- `Server_Game::handleUndoDraw`
- Uses `Context_UndoDraw`
- Moves card(s) back to deck

**Client:**
- Handled via `PlayerEventHandler::eventMoveCard`
- Logged as undo-draw context

---

### `FLIP_CARD` (1008)

**Purpose:** Flip a card face up or face down.

**Server:**
- `Server_Game::handleFlipCard`
- Updates card visibility
- Emits `Event_FlipCard`

**Client:**
- `PlayerEventHandler::eventFlipCard`

---

### `ATTACH_CARD` (1009)

**Purpose:** Attach one card to another.

**Server:**
- `Server_Game::handleAttachCard`
- Updates attachment graph
- Emits `Event_AttachCard`

**Client:**
- `PlayerEventHandler::eventAttachCard`

---

### `CREATE_TOKEN` (1010)

**Purpose:** Create a token card.

**Server:**
- `Server_Game::handleCreateToken`
- Allocates new card instance
- Emits `Event_CreateToken`

**Client:**
- `PlayerEventHandler::eventCreateToken`

---

### `CREATE_ARROW` (1011)

**Purpose:** Create a visual arrow.

**Server:**
- `Server_Game::handleCreateArrow`
- Registers arrow ownership
- Emits `Event_CreateArrow`

**Client:**
- `PlayerEventHandler::eventCreateArrow`

---

### `DELETE_ARROW` (1012)

**Purpose:** Remove a visual arrow.

**Server:**
- `Server_Game::handleDeleteArrow`
- Removes arrow
- Emits `Event_DeleteArrow`

**Client:**
- `PlayerEventHandler::eventDeleteArrow`

---

### `SET_CARD_ATTR` (1013)

**Purpose:** Set a card attribute.

**Server:**
- `Server_Game::handleSetCardAttr`
- Emits `Event_SetCardAttr`

**Client:**
- `PlayerEventHandler::eventSetCardAttr`

---

### `SET_CARD_COUNTER` (1014)

**Purpose:** Set a card counter.

**Server:**
- `Server_Game::handleSetCardCounter`
- Emits `Event_SetCardCounter`

**Client:**
- `PlayerEventHandler::eventSetCardCounter`

---

### `INC_CARD_COUNTER` (1015)

**Purpose:** Increment a card counter.

**Server:**
- `Server_Game::handleIncCardCounter`
- Normalized to set-counter
- Emits `Event_SetCardCounter`

**Client:**
- `PlayerEventHandler::eventSetCardCounter`

---

### `READY_START` (1016)

**Purpose:** Mark player as ready.

**Server:**
- `Server_Game::handleReadyStart`
- May call `doStartGameIfReady`

**Client:**
- Reflected via `Event_GameStateChanged`

---

### `CONCEDE` (1017)

**Purpose:** Concede the game.

**Server:**
- `Server_Game::handleConcede`
- Updates player state
- May end game

**Client:**
- Reflected via game state events

---

### `INC_COUNTER` (1018)

**Purpose:** Increment a global counter.

**Server:**
- `Server_Game::handleIncCounter`
- Emits `Event_SetCounter`

**Client:**
- `PlayerEventHandler::eventSetCounter`

---

### `CREATE_COUNTER` (1019)

**Purpose:** Create a global counter.

**Server:**
- `Server_Game::handleCreateCounter`
- Emits `Event_CreateCounter`

**Client:**
- `PlayerEventHandler::eventCreateCounter`

---

### `SET_COUNTER` (1020)

**Purpose:** Set a global counter value.

**Server:**
- `Server_Game::handleSetCounter`
- Emits `Event_SetCounter`

**Client:**
- `PlayerEventHandler::eventSetCounter`

---

### `DEL_COUNTER` (1021)

**Purpose:** Delete a global counter.

**Server:**
- `Server_Game::handleDelCounter`
- Emits `Event_DelCounter`

**Client:**
- `PlayerEventHandler::eventDelCounter`

---

### `NEXT_TURN` (1022)

**Purpose:** Advance to the next turn.

**Server:**
- `Server_Game::handleNextTurn`
- Emits:
  - `Event_SetActivePlayer`
  - `Event_SetActivePhase`

**Client:**
- Reflected via game state events

---

### `SET_ACTIVE_PHASE` (1023)

**Purpose:** Set active phase.

**Server:**
- `Server_Game::handleSetActivePhase`
- Emits `Event_SetActivePhase`

**Client:**
- Reflected via game state events

---

### `DUMP_ZONE` (1024)

**Purpose:** Dump zone contents.

**Server:**
- `Server_Game::handleDumpZone`
- Emits `Event_DumpZone`

**Client:**
- `PlayerEventHandler::eventDumpZone`

---

### `REVEAL_CARDS` (1026)

**Purpose:** Reveal specific cards.

**Server:**
- `Server_Game::handleRevealCards`
- Emits `Event_RevealCards`

**Client:**
- `PlayerEventHandler::eventRevealCards`

---

### `MOVE_CARD` (1027)

**Purpose:** Move a card between zones.

**Server:**
- `Server_Game::handleMoveCard`
- Emits `Event_MoveCard`
- May emit arrow cleanup events

**Client:**
- `PlayerEventHandler::eventMoveCard`

---

### `SET_SIDEBOARD_PLAN` (1028)

**Purpose:** Set sideboard configuration.

**Server:**
- `Server_Game::handleSetSideboardPlan`
- Stored server-side only

**Client:**
- Not forwarded as a game event

---

### `DECK_SELECT` (1029)

**Purpose:** Select a deck.

**Server:**
- `Server_Game::handleDeckSelect`

**Client:**
- Reflected via lobby / game state

---

### `SET_SIDEBOARD_LOCK` (1030)

**Purpose:** Lock or unlock sideboarding.

**Server:**
- `Server_Game::handleSetSideboardLock`

**Client:**
- Reflected via game state

---

### `CHANGE_ZONE_PROPERTIES` (1031)

**Purpose:** Change zone properties.

**Server:**
- `Server_Game::handleChangeZoneProperties`
- Emits `Event_ChangeZoneProperties`

**Client:**
- `PlayerEventHandler::eventChangeZoneProperties`

---

### `UNCONCEDE` (1032)

**Purpose:** Undo a concede.

**Server:**
- `Server_Game::handleUnconcede`

**Client:**
- Reflected via game state

---

### `JUDGE` (1033)

**Purpose:** Execute a command on behalf of another player.

**Server:**
- `Server_Game::handleJudge`
- Unwraps `Command_Judge`
- Dispatches embedded `GameCommand`

**Client:**
- Transparent; handled as normal commands

---

### `REVERSE_TURN` (1034)

**Purpose:** Reverse turn order.

**Server:**
- `Server_Game::handleReverseTurn`
- Toggles turn order flag

**Client:**
- Reflected via subsequent turn events

---

## Notes

- Only commands emitting `GameEvent`s appear in `PlayerEventHandler`
- Some commands affect game state without direct client events
- Judge commands are transport-level wrappers, not gameplay primitives
