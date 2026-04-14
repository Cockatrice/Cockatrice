import type { Event_AttachCard } from 'generated/proto/event_attach_card_pb';
import type { Event_ChangeZoneProperties } from 'generated/proto/event_change_zone_properties_pb';
import type { Event_CreateArrow } from 'generated/proto/event_create_arrow_pb';
import type { Event_CreateCounter } from 'generated/proto/event_create_counter_pb';
import type { Event_CreateToken } from 'generated/proto/event_create_token_pb';
import type { Event_DelCounter } from 'generated/proto/event_del_counter_pb';
import type { Event_DeleteArrow } from 'generated/proto/event_delete_arrow_pb';
import type { Event_DestroyCard } from 'generated/proto/event_destroy_card_pb';
import type { Event_DrawCards } from 'generated/proto/event_draw_cards_pb';
import type { Event_DumpZone } from 'generated/proto/event_dump_zone_pb';
import type { Event_FlipCard } from 'generated/proto/event_flip_card_pb';
import type { Event_GameStateChanged } from 'generated/proto/event_game_state_changed_pb';
import type { Event_MoveCard } from 'generated/proto/event_move_card_pb';
import type { ServerInfo_PlayerProperties } from 'generated/proto/serverinfo_playerproperties_pb';
import type { Event_RevealCards } from 'generated/proto/event_reveal_cards_pb';
import type { Event_RollDie } from 'generated/proto/event_roll_die_pb';
import type { Event_SetCardAttr } from 'generated/proto/event_set_card_attr_pb';
import type { Event_SetCardCounter } from 'generated/proto/event_set_card_counter_pb';
import type { Event_SetCounter } from 'generated/proto/event_set_counter_pb';
import type { Event_Shuffle } from 'generated/proto/event_shuffle_pb';
import { GameEntry } from './game.interfaces';
import { Types } from './game.types';

export const Actions = {
  clearStore: () => ({
    type: Types.CLEAR_STORE,
  }),

  gameJoined: (gameId: number, gameEntry: GameEntry) => ({
    type: Types.GAME_JOINED,
    gameId,
    gameEntry,
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

  gameStateChanged: (gameId: number, data: Event_GameStateChanged) => ({
    type: Types.GAME_STATE_CHANGED,
    gameId,
    data,
  }),

  playerJoined: (gameId: number, playerProperties: ServerInfo_PlayerProperties) => ({
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

  playerPropertiesChanged: (gameId: number, playerId: number, properties: ServerInfo_PlayerProperties) => ({
    type: Types.PLAYER_PROPERTIES_CHANGED,
    gameId,
    playerId,
    properties,
  }),

  kicked: (gameId: number) => ({
    type: Types.KICKED,
    gameId,
  }),

  cardMoved: (gameId: number, playerId: number, data: Event_MoveCard) => ({
    type: Types.CARD_MOVED,
    gameId,
    playerId,
    data,
  }),

  cardFlipped: (gameId: number, playerId: number, data: Event_FlipCard) => ({
    type: Types.CARD_FLIPPED,
    gameId,
    playerId,
    data,
  }),

  cardDestroyed: (gameId: number, playerId: number, data: Event_DestroyCard) => ({
    type: Types.CARD_DESTROYED,
    gameId,
    playerId,
    data,
  }),

  cardAttached: (gameId: number, playerId: number, data: Event_AttachCard) => ({
    type: Types.CARD_ATTACHED,
    gameId,
    playerId,
    data,
  }),

  tokenCreated: (gameId: number, playerId: number, data: Event_CreateToken) => ({
    type: Types.TOKEN_CREATED,
    gameId,
    playerId,
    data,
  }),

  cardAttrChanged: (gameId: number, playerId: number, data: Event_SetCardAttr) => ({
    type: Types.CARD_ATTR_CHANGED,
    gameId,
    playerId,
    data,
  }),

  cardCounterChanged: (gameId: number, playerId: number, data: Event_SetCardCounter) => ({
    type: Types.CARD_COUNTER_CHANGED,
    gameId,
    playerId,
    data,
  }),

  arrowCreated: (gameId: number, playerId: number, data: Event_CreateArrow) => ({
    type: Types.ARROW_CREATED,
    gameId,
    playerId,
    data,
  }),

  arrowDeleted: (gameId: number, playerId: number, data: Event_DeleteArrow) => ({
    type: Types.ARROW_DELETED,
    gameId,
    playerId,
    data,
  }),

  counterCreated: (gameId: number, playerId: number, data: Event_CreateCounter) => ({
    type: Types.COUNTER_CREATED,
    gameId,
    playerId,
    data,
  }),

  counterSet: (gameId: number, playerId: number, data: Event_SetCounter) => ({
    type: Types.COUNTER_SET,
    gameId,
    playerId,
    data,
  }),

  counterDeleted: (gameId: number, playerId: number, data: Event_DelCounter) => ({
    type: Types.COUNTER_DELETED,
    gameId,
    playerId,
    data,
  }),

  cardsDrawn: (gameId: number, playerId: number, data: Event_DrawCards) => ({
    type: Types.CARDS_DRAWN,
    gameId,
    playerId,
    data,
  }),

  cardsRevealed: (gameId: number, playerId: number, data: Event_RevealCards) => ({
    type: Types.CARDS_REVEALED,
    gameId,
    playerId,
    data,
  }),

  zoneShuffled: (gameId: number, playerId: number, data: Event_Shuffle) => ({
    type: Types.ZONE_SHUFFLED,
    gameId,
    playerId,
    data,
  }),

  dieRolled: (gameId: number, playerId: number, data: Event_RollDie) => ({
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

  zoneDumped: (gameId: number, playerId: number, data: Event_DumpZone) => ({
    type: Types.ZONE_DUMPED,
    gameId,
    playerId,
    data,
  }),

  zonePropertiesChanged: (gameId: number, playerId: number, data: Event_ChangeZoneProperties) => ({
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
