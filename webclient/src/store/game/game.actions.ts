import type { Data } from '@app/types';
import { Types } from './game.types';

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE,
  }),

  gameJoined: (data: Data.Event_GameJoined) => ({
    type: Types.GAME_JOINED,
    data,
  }),

  gameLeft: (gameId: number) => ({
    type: Types.GAME_LEFT,
    gameId,
  }),

  gameClosed: (gameId: number) => ({
    type: Types.GAME_CLOSED,
    gameId,
  }),

  gameHostChanged: (gameId: number, hostId: number) => ({
    type: Types.GAME_HOST_CHANGED,
    gameId,
    hostId,
  }),

  gameStateChanged: (gameId: number, data: Data.Event_GameStateChanged) => ({
    type: Types.GAME_STATE_CHANGED,
    gameId,
    data,
  }),

  playerJoined: (gameId: number, playerProperties: Data.ServerInfo_PlayerProperties) => ({
    type: Types.PLAYER_JOINED,
    gameId,
    playerProperties,
  }),

  playerLeft: (gameId: number, playerId: number, reason: number) => ({
    type: Types.PLAYER_LEFT,
    gameId,
    playerId,
    reason,
  }),

  playerPropertiesChanged: (gameId: number, playerId: number, properties: Data.ServerInfo_PlayerProperties) => ({
    type: Types.PLAYER_PROPERTIES_CHANGED,
    gameId,
    playerId,
    properties,
  }),

  kicked: (gameId: number) => ({
    type: Types.KICKED,
    gameId,
  }),

  cardMoved: (gameId: number, playerId: number, data: Data.Event_MoveCard) => ({
    type: Types.CARD_MOVED,
    gameId,
    playerId,
    data,
  }),

  cardFlipped: (gameId: number, playerId: number, data: Data.Event_FlipCard) => ({
    type: Types.CARD_FLIPPED,
    gameId,
    playerId,
    data,
  }),

  cardDestroyed: (gameId: number, playerId: number, data: Data.Event_DestroyCard) => ({
    type: Types.CARD_DESTROYED,
    gameId,
    playerId,
    data,
  }),

  cardAttached: (gameId: number, playerId: number, data: Data.Event_AttachCard) => ({
    type: Types.CARD_ATTACHED,
    gameId,
    playerId,
    data,
  }),

  tokenCreated: (gameId: number, playerId: number, data: Data.Event_CreateToken) => ({
    type: Types.TOKEN_CREATED,
    gameId,
    playerId,
    data,
  }),

  cardAttrChanged: (gameId: number, playerId: number, data: Data.Event_SetCardAttr) => ({
    type: Types.CARD_ATTR_CHANGED,
    gameId,
    playerId,
    data,
  }),

  cardCounterChanged: (gameId: number, playerId: number, data: Data.Event_SetCardCounter) => ({
    type: Types.CARD_COUNTER_CHANGED,
    gameId,
    playerId,
    data,
  }),

  arrowCreated: (gameId: number, playerId: number, data: Data.Event_CreateArrow) => ({
    type: Types.ARROW_CREATED,
    gameId,
    playerId,
    data,
  }),

  arrowDeleted: (gameId: number, playerId: number, data: Data.Event_DeleteArrow) => ({
    type: Types.ARROW_DELETED,
    gameId,
    playerId,
    data,
  }),

  counterCreated: (gameId: number, playerId: number, data: Data.Event_CreateCounter) => ({
    type: Types.COUNTER_CREATED,
    gameId,
    playerId,
    data,
  }),

  counterSet: (gameId: number, playerId: number, data: Data.Event_SetCounter) => ({
    type: Types.COUNTER_SET,
    gameId,
    playerId,
    data,
  }),

  counterDeleted: (gameId: number, playerId: number, data: Data.Event_DelCounter) => ({
    type: Types.COUNTER_DELETED,
    gameId,
    playerId,
    data,
  }),

  cardsDrawn: (gameId: number, playerId: number, data: Data.Event_DrawCards) => ({
    type: Types.CARDS_DRAWN,
    gameId,
    playerId,
    data,
  }),

  cardsRevealed: (gameId: number, playerId: number, data: Data.Event_RevealCards) => ({
    type: Types.CARDS_REVEALED,
    gameId,
    playerId,
    data,
  }),

  zoneShuffled: (gameId: number, playerId: number, data: Data.Event_Shuffle) => ({
    type: Types.ZONE_SHUFFLED,
    gameId,
    playerId,
    data,
  }),

  dieRolled: (gameId: number, playerId: number, data: Data.Event_RollDie) => ({
    type: Types.DIE_ROLLED,
    gameId,
    playerId,
    data,
  }),

  activePlayerSet: (gameId: number, activePlayerId: number) => ({
    type: Types.ACTIVE_PLAYER_SET,
    gameId,
    activePlayerId,
  }),

  activePhaseSet: (gameId: number, phase: number) => ({
    type: Types.ACTIVE_PHASE_SET,
    gameId,
    phase,
  }),

  turnReversed: (gameId: number, reversed: boolean) => ({
    type: Types.TURN_REVERSED,
    gameId,
    reversed,
  }),

  zoneDumped: (gameId: number, playerId: number, data: Data.Event_DumpZone) => ({
    type: Types.ZONE_DUMPED,
    gameId,
    playerId,
    data,
  }),

  zonePropertiesChanged: (gameId: number, playerId: number, data: Data.Event_ChangeZoneProperties) => ({
    type: Types.ZONE_PROPERTIES_CHANGED,
    gameId,
    playerId,
    data,
  }),

  gameSay: (gameId: number, playerId: number, message: string) => ({
    type: Types.GAME_SAY,
    gameId,
    playerId,
    message,
  }),
};

export type GameAction = ReturnType<typeof Actions[keyof typeof Actions]>;
