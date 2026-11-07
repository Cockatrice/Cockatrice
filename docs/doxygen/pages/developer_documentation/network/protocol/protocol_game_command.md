@page protocol_game_command GameCommand (Protocol Concept)

# Game Commands Reference

This document describes game-level commands (`GameCommandType`) and how they are
handled across the server and client.

Flow overview:

Client
→ GameCommand
→ Server command handler (`Server_Player`, `Server_AbstractParticipant`, or `Server_Game`)
→ GameEvent(s)
→ PlayerEventHandler::event*

## Command Mapping

### `GAME_SAY` (1002)

**Purpose:** Send a chat message during a game.

**Server:**
- `Server_AbstractParticipant::cmdGameSay`
- Validates spectator chat permissions
- Applies chat flood protection
- Emits `Event_GameSay`
- Logs the message to the server database

**Client:**
- `PlayerEventHandler::eventGameSay`
- Adds the chat message to the game log

---

### `SHUFFLE` (1003)

**Purpose:** Shuffle a card zone (usually library).

**Server:**
- `Server_Player::cmdShuffle`
- Only allows shuffling the deck zone
- Rejects if the game has not started or the player has conceded
- Supports partial-range shuffles (`start` / `end`)
- Emits `Event_Shuffle`
- Re-reveals the top card if the deck is being tracked

**Client:**
- `PlayerEventHandler::eventShuffle`
- Closes affected library views
- Clears any revealed top card if necessary
- Updates deck graphics
- Logs the shuffle

---

### `ROLL_DIE` (1005)

**Purpose:** Roll one or more dice.

**Server:**
- Computes random die results
- Emits `Event_RollDie`

**Client:**
- `PlayerEventHandler::eventRollDie`
- Displays the sorted roll results in the game log

---

### `DRAW_CARDS` (1006)

**Purpose:** Draw cards from deck.

**Server:**
- `Server_Player::cmdDrawCards`
- Rejects if the game has not started or the player has conceded
- Moves cards from deck to hand
- Sends full card information privately to the drawing player
- Sends only the draw count to all other players
- Tracks drawn cards for undo
- Updates revealed-top-card state when necessary
- Emits `Event_DrawCards`

**Client:**
- `PlayerEventHandler::eventDrawCards`
- Moves cards from library to hand
- Reveals card identities only when included in the event
- Updates both zones
- Logs the draw

---

### `UNDO_DRAW` (1007)

**Purpose:** Undo a previous draw.

**Server:**
- `Server_Player::cmdUndoDraw`
- Rejects if the game has not started or the player has conceded
- Uses the tracked draw history (`lastDrawList`)
- Moves the most recently drawable card from hand back to the top of the deck
- Emits `Event_GameLogNotice` if undo is no longer possible

**Client:**
- Processed as a normal `MOVE_CARD`
- Detects `Context_UndoDraw`
- Logs the undo draw instead of a normal move

---

### `FLIP_CARD` (1008)

**Purpose:** Flip a card face up or face down.

**Server:**
- Updates the card's face-down state
- Reveals identity when turning face up
- Emits `Event_FlipCard`

**Client:**
- `PlayerEventHandler::eventFlipCard`
- Updates the card identity when revealed
- Changes face-up / face-down state
- Updates card menus
- Logs the flip

---

### `ATTACH_CARD` (1009)

**Purpose:** Attach one card to another.

**Server:**
- Updates attachment relationships
- Emits `Event_AttachCard`

**Client:**
- `PlayerEventHandler::eventAttachCard`
- Updates parent/child attachment links
- Reorganizes affected zones
- Logs attach or detach operations
- Refreshes card actions

---

### `CREATE_TOKEN` (1010)

**Purpose:** Create a token card.

**Server:**
- Creates a new token card
- Emits `Event_CreateToken`

**Client:**
- `PlayerEventHandler::eventCreateToken`
- Creates the token object
- Applies token properties (PT, color, annotation, face-down state)
- Adds it to the requested zone
- Logs token creation

---

### `CREATE_ARROW` (1011)

**Purpose:** Create a visual arrow.

**Server:**
- Creates a visual arrow
- Emits `Event_CreateArrow`

**Client:**
- `PlayerEventHandler::eventCreateArrow`
- Creates the arrow graphics
- Resolves endpoint card names
- Logs arrow creation

---

### `DELETE_ARROW` (1012)

**Purpose:** Remove a visual arrow.

**Server:**
- Removes an existing arrow
- Emits `Event_DeleteArrow`

**Client:**
- `PlayerEventHandler::eventDeleteArrow`
- Removes the arrow graphics

---

### `SET_CARD_ATTR` (1013)

**Purpose:** Set a card attribute.

**Server:**
- Updates one or more card attributes
- Emits `Event_SetCardAttr`

**Client:**
- `PlayerEventHandler::eventSetCardAttr`
- Updates tapped state, color, annotation, power/toughness, face-down state, attacking state, and related visuals
- Logs attribute changes where appropriate

---

### `SET_CARD_COUNTER` (1014)

**Purpose:** Set a card counter.

**Server:**
- Updates the specified card counter
- Emits `Event_SetCardCounter`

**Client:**
- `PlayerEventHandler::eventSetCardCounter`
- Updates the displayed counter value
- Refreshes card actions
- Logs the counter change

---

### `INC_CARD_COUNTER` (1015)

**Purpose:** Increment a card counter.

**Server:**
- Normalizes to a set-counter operation
- Emits `Event_SetCardCounter`

**Client:**
- Processed identically to `SET_CARD_COUNTER`

---

### `READY_START` (1016)

**Purpose:** Mark player as ready.

**Server:**
- Marks the player as ready
- Starts the game once all required players are ready

**Client:**
- Reflected through updated game state events

---

### `CONCEDE` (1017)

**Purpose:** Concede the game.

**Server:**
- Marks the player as conceded
- Returns borrowed cards where appropriate
- Ends the game if only one player remains

**Client:**
- Reflected through updated game state events

---

### `INC_COUNTER` (1018)

**Purpose:** Increment a player counter.

**Server:**
- `Server_Player::cmdIncCounter`
- Rejects if the game has not started or the player has conceded
- Updates the counter value
- Emits `Event_SetCounter` only if the value changed

**Client:**
- `PlayerEventHandler::eventSetCounter`
- Updates the displayed player counter
- Logs the change

---

### `CREATE_COUNTER` (1019)

**Purpose:** Create a player counter.

**Server:**
- `Server_Player::cmdCreateCounter`
- Rejects if the game has not started or the player has conceded
- Allocates a new counter ID
- Creates the counter
- Emits `Event_CreateCounter`

**Client:**
- `PlayerEventHandler::eventCreateCounter`
- Creates the local player counter

---

### `SET_COUNTER` (1020)

**Purpose:** Set a player counter value.

**Server:**
- `Server_Player::cmdSetCounter`
- Rejects if the game has not started or the player has conceded
- Updates the counter value
- Emits `Event_SetCounter` only if the value changed

**Client:**
- `PlayerEventHandler::eventSetCounter`
- Updates the counter value
- Logs the change

---

### `DEL_COUNTER` (1021)

**Purpose:** Delete a player counter.

**Server:**
- `Server_Player::cmdDelCounter`
- Rejects if the game has not started or the player has conceded
- Deletes the counter
- Emits `Event_DelCounter`

**Client:**
- `PlayerEventHandler::eventDelCounter`
- Removes the counter

---

### `NEXT_TURN` (1022)

**Purpose:** Advance to the next turn.

**Server:**
- `Server_Player::cmdNextTurn`
- Rejects if the game has not started
- Conceded players cannot advance turns unless acting as a judge
- Calls `Server_Game::nextTurn()`

**Client:**
- Reflected through subsequent active-player and active-phase events

---

### `SET_ACTIVE_PHASE` (1023)

**Purpose:** Set active phase.

**Server:**
- `Server_Player::cmdSetActivePhase`
- Rejects if the game has not started
- Judges may change the phase at any time
- Normal players may only change the phase during their own active turn
- Calls `Server_Game::setActivePhase()`

**Client:**
- Reflected through updated phase events

---

### `DUMP_ZONE` (1024)

**Purpose:** Dump zone contents.

**Server:**
- Generates a zone summary
- Emits `Event_DumpZone`

**Client:**
- `PlayerEventHandler::eventDumpZone`
- Logs the zone contents summary

---

### `REVEAL_CARDS` (1026)

**Purpose:** Reveal specific cards.

**Server:**
- Reveals cards to one or more players
- Emits `Event_RevealCards`

**Client:**
- `PlayerEventHandler::eventRevealCards`
- Reveals cards in-place or in a reveal window
- Supports temporary peeks
- Updates revealed top cards
- Logs the reveal

---

### `MOVE_CARD` (1027)

**Purpose:** Move a card between zones.

**Server:**
- Moves cards between zones
- Updates ownership and attachments as needed
- Emits `Event_MoveCard`

**Client:**
- `PlayerEventHandler::eventMoveCard`
- Moves the card between zones
- Updates ownership, attachments, IDs and revealed information
- Handles undo-draw context
- Refreshes menus and zone layouts
- Logs the move

---

### `SET_SIDEBOARD_PLAN` (1028)

**Purpose:** Set sideboard configuration.

**Server:**
- `Server_Player::cmdSetSideboardPlan`
- Allowed only before readying for the game
- Requires a loaded deck
- Requires sideboarding to be unlocked
- Stores the current sideboard plan

**Client:**
- No game event is generated

---

### `DECK_SELECT` (1029)

**Purpose:** Select a deck.

**Server:**
- `Server_Player::cmdDeckSelect`
- Allowed only before the game starts
- Loads a deck from either the database or serialized deck data
- Locks sideboarding
- Updates player properties
- Emits `Event_PlayerPropertiesChanged`
- Attaches `Context_DeckSelect`
- Returns the selected deck in the command response

**Client:**
- Reflected through updated player properties and command response

---

### `SET_SIDEBOARD_LOCK` (1030)

**Purpose:** Lock or unlock sideboarding.

**Server:**
- `Server_Player::cmdSetSideboardLock`
- Allowed only before readying
- Toggles sideboard locking
- Clears pending sideboard plans when locking
- Emits `Event_PlayerPropertiesChanged`
- Attaches `Context_SetSideboardLock`

**Client:**
- Reflected through updated player properties

---

### `CHANGE_ZONE_PROPERTIES` (1031)

**Purpose:** Change zone properties.

**Server:**
- `Server_Player::cmdChangeZoneProperties`
- Delegates to the abstract implementation
- Updates top-card revelation if required
- Emits `Event_ChangeZoneProperties`

**Client:**
- `PlayerEventHandler::eventChangeZoneProperties`
- Updates always-reveal-top-card and always-look-at-top-card settings
- Logs property changes

---

### `UNCONCEDE` (1032)

**Purpose:** Undo a concede.

**Server:**
- Restores the player's active status

**Client:**
- Reflected through updated game state events

---

### `JUDGE` (1033)

**Purpose:** Execute commands on behalf of another player.

**Server:**
- `Server_AbstractParticipant::cmdJudge`
- Requires judge privileges
- Executes embedded commands as the selected player
- Marks resulting events as judge-forced

**Client:**
- Transparent; resulting events are processed normally

---

### `REVERSE_TURN` (1034)

**Purpose:** Reverse turn order.

**Server:**
- `Server_Player::cmdReverseTurn`
- Conceded non-judge players cannot use the command
- Delegates to `Server_AbstractParticipant::cmdReverseTurn`
- Toggles turn order
- Emits `Event_ReverseTurn`

**Client:**
- Reflected by subsequent active-player progression

---

## Notes

- Game commands are handled by `Server_Player`, `Server_AbstractParticipant`, or `Server_Game`, depending on the command.
- Only commands that emit `GameEvent`s produce corresponding `PlayerEventHandler` callbacks.
- Some commands modify server state without generating a dedicated client event.
- Judge commands are transport wrappers that execute other game commands on behalf of another player.