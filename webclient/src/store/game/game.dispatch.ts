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
import { store } from 'store/store';
import { Actions } from './game.actions';
import { GameEntry } from './game.interfaces';

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },

  gameJoined: (gameId: number, gameEntry: GameEntry) => {
    store.dispatch(Actions.gameJoined(gameId, gameEntry));
  },

  gameLeft: (gameId: number) => {
    store.dispatch(Actions.gameLeft(gameId));
  },

  gameClosed: (gameId: number) => {
    store.dispatch(Actions.gameClosed(gameId));
  },

  gameHostChanged: (gameId: number, hostId: number) => {
    store.dispatch(Actions.gameHostChanged(gameId, hostId));
  },

  gameStateChanged: (gameId: number, data: Event_GameStateChanged) => {
    store.dispatch(Actions.gameStateChanged(gameId, data));
  },

  playerJoined: (gameId: number, playerProperties: ServerInfo_PlayerProperties) => {
    store.dispatch(Actions.playerJoined(gameId, playerProperties));
  },

  playerLeft: (gameId: number, playerId: number, reason: number) => {
    store.dispatch(Actions.playerLeft(gameId, playerId, reason));
  },

  playerPropertiesChanged: (gameId: number, playerId: number, properties: ServerInfo_PlayerProperties) => {
    store.dispatch(Actions.playerPropertiesChanged(gameId, playerId, properties));
  },

  kicked: (gameId: number) => {
    store.dispatch(Actions.kicked(gameId));
  },

  cardMoved: (gameId: number, playerId: number, data: Event_MoveCard) => {
    store.dispatch(Actions.cardMoved(gameId, playerId, data));
  },

  cardFlipped: (gameId: number, playerId: number, data: Event_FlipCard) => {
    store.dispatch(Actions.cardFlipped(gameId, playerId, data));
  },

  cardDestroyed: (gameId: number, playerId: number, data: Event_DestroyCard) => {
    store.dispatch(Actions.cardDestroyed(gameId, playerId, data));
  },

  cardAttached: (gameId: number, playerId: number, data: Event_AttachCard) => {
    store.dispatch(Actions.cardAttached(gameId, playerId, data));
  },

  tokenCreated: (gameId: number, playerId: number, data: Event_CreateToken) => {
    store.dispatch(Actions.tokenCreated(gameId, playerId, data));
  },

  cardAttrChanged: (gameId: number, playerId: number, data: Event_SetCardAttr) => {
    store.dispatch(Actions.cardAttrChanged(gameId, playerId, data));
  },

  cardCounterChanged: (gameId: number, playerId: number, data: Event_SetCardCounter) => {
    store.dispatch(Actions.cardCounterChanged(gameId, playerId, data));
  },

  arrowCreated: (gameId: number, playerId: number, data: Event_CreateArrow) => {
    store.dispatch(Actions.arrowCreated(gameId, playerId, data));
  },

  arrowDeleted: (gameId: number, playerId: number, data: Event_DeleteArrow) => {
    store.dispatch(Actions.arrowDeleted(gameId, playerId, data));
  },

  counterCreated: (gameId: number, playerId: number, data: Event_CreateCounter) => {
    store.dispatch(Actions.counterCreated(gameId, playerId, data));
  },

  counterSet: (gameId: number, playerId: number, data: Event_SetCounter) => {
    store.dispatch(Actions.counterSet(gameId, playerId, data));
  },

  counterDeleted: (gameId: number, playerId: number, data: Event_DelCounter) => {
    store.dispatch(Actions.counterDeleted(gameId, playerId, data));
  },

  cardsDrawn: (gameId: number, playerId: number, data: Event_DrawCards) => {
    store.dispatch(Actions.cardsDrawn(gameId, playerId, data));
  },

  cardsRevealed: (gameId: number, playerId: number, data: Event_RevealCards) => {
    store.dispatch(Actions.cardsRevealed(gameId, playerId, data));
  },

  zoneShuffled: (gameId: number, playerId: number, data: Event_Shuffle) => {
    store.dispatch(Actions.zoneShuffled(gameId, playerId, data));
  },

  dieRolled: (gameId: number, playerId: number, data: Event_RollDie) => {
    store.dispatch(Actions.dieRolled(gameId, playerId, data));
  },

  activePlayerSet: (gameId: number, activePlayerId: number) => {
    store.dispatch(Actions.activePlayerSet(gameId, activePlayerId));
  },

  activePhaseSet: (gameId: number, phase: number) => {
    store.dispatch(Actions.activePhaseSet(gameId, phase));
  },

  turnReversed: (gameId: number, reversed: boolean) => {
    store.dispatch(Actions.turnReversed(gameId, reversed));
  },

  zoneDumped: (gameId: number, playerId: number, data: Event_DumpZone) => {
    store.dispatch(Actions.zoneDumped(gameId, playerId, data));
  },

  zonePropertiesChanged: (gameId: number, playerId: number, data: Event_ChangeZoneProperties) => {
    store.dispatch(Actions.zonePropertiesChanged(gameId, playerId, data));
  },

  gameSay: (gameId: number, playerId: number, message: string) => {
    store.dispatch(Actions.gameSay(gameId, playerId, message));
  },
};
