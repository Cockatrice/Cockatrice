import type { Data } from '@app/types';
import { store } from '..';
import { Actions } from './game.actions';

export const Dispatch = {
  clearStore: () => {
    store.dispatch(Actions.clearStore());
  },

  gameJoined: (data: Data.Event_GameJoined) => {
    store.dispatch(Actions.gameJoined({ data }));
  },

  gameLeft: (gameId: number) => {
    store.dispatch(Actions.gameLeft({ gameId }));
  },

  gameClosed: (gameId: number) => {
    store.dispatch(Actions.gameClosed({ gameId }));
  },

  gameHostChanged: (gameId: number, hostId: number) => {
    store.dispatch(Actions.gameHostChanged({ gameId, hostId }));
  },

  gameStateChanged: (gameId: number, data: Data.Event_GameStateChanged) => {
    store.dispatch(Actions.gameStateChanged({ gameId, data }));
  },

  playerJoined: (gameId: number, playerProperties: Data.ServerInfo_PlayerProperties) => {
    store.dispatch(Actions.playerJoined({ gameId, playerProperties }));
  },

  playerLeft: (gameId: number, playerId: number, _reason: number) => {
    store.dispatch(Actions.playerLeft({ gameId, playerId }));
  },

  playerPropertiesChanged: (gameId: number, playerId: number, properties: Data.ServerInfo_PlayerProperties) => {
    store.dispatch(Actions.playerPropertiesChanged({ gameId, playerId, properties }));
  },

  kicked: (gameId: number) => {
    store.dispatch(Actions.kicked({ gameId }));
  },

  cardMoved: (gameId: number, playerId: number, data: Data.Event_MoveCard) => {
    store.dispatch(Actions.cardMoved({ gameId, playerId, data }));
  },

  cardFlipped: (gameId: number, playerId: number, data: Data.Event_FlipCard) => {
    store.dispatch(Actions.cardFlipped({ gameId, playerId, data }));
  },

  cardDestroyed: (gameId: number, playerId: number, data: Data.Event_DestroyCard) => {
    store.dispatch(Actions.cardDestroyed({ gameId, playerId, data }));
  },

  cardAttached: (gameId: number, playerId: number, data: Data.Event_AttachCard) => {
    store.dispatch(Actions.cardAttached({ gameId, playerId, data }));
  },

  tokenCreated: (gameId: number, playerId: number, data: Data.Event_CreateToken) => {
    store.dispatch(Actions.tokenCreated({ gameId, playerId, data }));
  },

  cardAttrChanged: (gameId: number, playerId: number, data: Data.Event_SetCardAttr) => {
    store.dispatch(Actions.cardAttrChanged({ gameId, playerId, data }));
  },

  cardCounterChanged: (gameId: number, playerId: number, data: Data.Event_SetCardCounter) => {
    store.dispatch(Actions.cardCounterChanged({ gameId, playerId, data }));
  },

  arrowCreated: (gameId: number, playerId: number, data: Data.Event_CreateArrow) => {
    store.dispatch(Actions.arrowCreated({ gameId, playerId, data }));
  },

  arrowDeleted: (gameId: number, playerId: number, data: Data.Event_DeleteArrow) => {
    store.dispatch(Actions.arrowDeleted({ gameId, playerId, data }));
  },

  counterCreated: (gameId: number, playerId: number, data: Data.Event_CreateCounter) => {
    store.dispatch(Actions.counterCreated({ gameId, playerId, data }));
  },

  counterSet: (gameId: number, playerId: number, data: Data.Event_SetCounter) => {
    store.dispatch(Actions.counterSet({ gameId, playerId, data }));
  },

  counterDeleted: (gameId: number, playerId: number, data: Data.Event_DelCounter) => {
    store.dispatch(Actions.counterDeleted({ gameId, playerId, data }));
  },

  cardsDrawn: (gameId: number, playerId: number, data: Data.Event_DrawCards) => {
    store.dispatch(Actions.cardsDrawn({ gameId, playerId, data }));
  },

  cardsRevealed: (gameId: number, playerId: number, data: Data.Event_RevealCards) => {
    store.dispatch(Actions.cardsRevealed({ gameId, playerId, data }));
  },

  zoneShuffled: (gameId: number, playerId: number, data: Data.Event_Shuffle) => {
    store.dispatch(Actions.zoneShuffled({ gameId, playerId, data }));
  },

  dieRolled: (gameId: number, playerId: number, data: Data.Event_RollDie) => {
    store.dispatch(Actions.dieRolled({ gameId, playerId, data }));
  },

  activePlayerSet: (gameId: number, activePlayerId: number) => {
    store.dispatch(Actions.activePlayerSet({ gameId, activePlayerId }));
  },

  activePhaseSet: (gameId: number, phase: number) => {
    store.dispatch(Actions.activePhaseSet({ gameId, phase }));
  },

  turnReversed: (gameId: number, reversed: boolean) => {
    store.dispatch(Actions.turnReversed({ gameId, reversed }));
  },

  zoneDumped: (gameId: number, playerId: number, data: Data.Event_DumpZone) => {
    store.dispatch(Actions.zoneDumped({ gameId, playerId, data }));
  },

  zonePropertiesChanged: (gameId: number, playerId: number, data: Data.Event_ChangeZoneProperties) => {
    store.dispatch(Actions.zonePropertiesChanged({ gameId, playerId, data }));
  },

  gameSay: (gameId: number, playerId: number, message: string, timeReceived: number) => {
    store.dispatch(Actions.gameSay({ gameId, playerId, message, timeReceived }));
  },
};
