import { gamesSlice } from './game.reducer';

const a = gamesSlice.actions;

export const Types = {
  CLEAR_STORE: a.clearStore.type,
  GAME_JOINED: a.gameJoined.type,
  GAME_LEFT: a.gameLeft.type,
  GAME_CLOSED: a.gameClosed.type,
  GAME_HOST_CHANGED: a.gameHostChanged.type,
  GAME_STATE_CHANGED: a.gameStateChanged.type,
  PLAYER_JOINED: a.playerJoined.type,
  PLAYER_LEFT: a.playerLeft.type,
  PLAYER_PROPERTIES_CHANGED: a.playerPropertiesChanged.type,
  KICKED: a.kicked.type,
  CARD_MOVED: a.cardMoved.type,
  CARD_FLIPPED: a.cardFlipped.type,
  CARD_DESTROYED: a.cardDestroyed.type,
  CARD_ATTACHED: a.cardAttached.type,
  TOKEN_CREATED: a.tokenCreated.type,
  CARD_ATTR_CHANGED: a.cardAttrChanged.type,
  CARD_COUNTER_CHANGED: a.cardCounterChanged.type,
  ARROW_CREATED: a.arrowCreated.type,
  ARROW_DELETED: a.arrowDeleted.type,
  COUNTER_CREATED: a.counterCreated.type,
  COUNTER_SET: a.counterSet.type,
  COUNTER_DELETED: a.counterDeleted.type,
  CARDS_DRAWN: a.cardsDrawn.type,
  CARDS_REVEALED: a.cardsRevealed.type,
  ZONE_SHUFFLED: a.zoneShuffled.type,
  DIE_ROLLED: a.dieRolled.type,
  ACTIVE_PLAYER_SET: a.activePlayerSet.type,
  ACTIVE_PHASE_SET: a.activePhaseSet.type,
  TURN_REVERSED: a.turnReversed.type,
  ZONE_DUMPED: a.zoneDumped.type,
  ZONE_PROPERTIES_CHANGED: a.zonePropertiesChanged.type,
  GAME_SAY: a.gameSay.type,
} as const;

export { MAX_GAME_MESSAGES } from './game.reducer';
