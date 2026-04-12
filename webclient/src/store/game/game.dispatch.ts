import {
  AttachCardData,
  ChangeZonePropertiesData,
  CreateArrowData,
  CreateCounterData,
  CreateTokenData,
  DelCounterData,
  DeleteArrowData,
  DestroyCardData,
  DrawCardsData,
  DumpZoneData,
  FlipCardData,
  GameStateChangedData,
  MoveCardData,
  PlayerProperties,
  RevealCardsData,
  RollDieData,
  SetCardAttrData,
  SetCardCounterData,
  SetCounterData,
  ShuffleData,
} from 'types';
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

  gameStateChanged: (gameId: number, data: GameStateChangedData) => {
    store.dispatch(Actions.gameStateChanged(gameId, data));
  },

  playerJoined: (gameId: number, playerProperties: PlayerProperties) => {
    store.dispatch(Actions.playerJoined(gameId, playerProperties));
  },

  playerLeft: (gameId: number, playerId: number, reason: number) => {
    store.dispatch(Actions.playerLeft(gameId, playerId, reason));
  },

  playerPropertiesChanged: (gameId: number, playerId: number, properties: PlayerProperties) => {
    store.dispatch(Actions.playerPropertiesChanged(gameId, playerId, properties));
  },

  kicked: (gameId: number) => {
    store.dispatch(Actions.kicked(gameId));
  },

  cardMoved: (gameId: number, playerId: number, data: MoveCardData) => {
    store.dispatch(Actions.cardMoved(gameId, playerId, data));
  },

  cardFlipped: (gameId: number, playerId: number, data: FlipCardData) => {
    store.dispatch(Actions.cardFlipped(gameId, playerId, data));
  },

  cardDestroyed: (gameId: number, playerId: number, data: DestroyCardData) => {
    store.dispatch(Actions.cardDestroyed(gameId, playerId, data));
  },

  cardAttached: (gameId: number, playerId: number, data: AttachCardData) => {
    store.dispatch(Actions.cardAttached(gameId, playerId, data));
  },

  tokenCreated: (gameId: number, playerId: number, data: CreateTokenData) => {
    store.dispatch(Actions.tokenCreated(gameId, playerId, data));
  },

  cardAttrChanged: (gameId: number, playerId: number, data: SetCardAttrData) => {
    store.dispatch(Actions.cardAttrChanged(gameId, playerId, data));
  },

  cardCounterChanged: (gameId: number, playerId: number, data: SetCardCounterData) => {
    store.dispatch(Actions.cardCounterChanged(gameId, playerId, data));
  },

  arrowCreated: (gameId: number, playerId: number, data: CreateArrowData) => {
    store.dispatch(Actions.arrowCreated(gameId, playerId, data));
  },

  arrowDeleted: (gameId: number, playerId: number, data: DeleteArrowData) => {
    store.dispatch(Actions.arrowDeleted(gameId, playerId, data));
  },

  counterCreated: (gameId: number, playerId: number, data: CreateCounterData) => {
    store.dispatch(Actions.counterCreated(gameId, playerId, data));
  },

  counterSet: (gameId: number, playerId: number, data: SetCounterData) => {
    store.dispatch(Actions.counterSet(gameId, playerId, data));
  },

  counterDeleted: (gameId: number, playerId: number, data: DelCounterData) => {
    store.dispatch(Actions.counterDeleted(gameId, playerId, data));
  },

  cardsDrawn: (gameId: number, playerId: number, data: DrawCardsData) => {
    store.dispatch(Actions.cardsDrawn(gameId, playerId, data));
  },

  cardsRevealed: (gameId: number, playerId: number, data: RevealCardsData) => {
    store.dispatch(Actions.cardsRevealed(gameId, playerId, data));
  },

  zoneShuffled: (gameId: number, playerId: number, data: ShuffleData) => {
    store.dispatch(Actions.zoneShuffled(gameId, playerId, data));
  },

  dieRolled: (gameId: number, playerId: number, data: RollDieData) => {
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

  zoneDumped: (gameId: number, playerId: number, data: DumpZoneData) => {
    store.dispatch(Actions.zoneDumped(gameId, playerId, data));
  },

  zonePropertiesChanged: (gameId: number, playerId: number, data: ChangeZonePropertiesData) => {
    store.dispatch(Actions.zonePropertiesChanged(gameId, playerId, data));
  },

  gameSay: (gameId: number, playerId: number, message: string) => {
    store.dispatch(Actions.gameSay(gameId, playerId, message));
  },
};
