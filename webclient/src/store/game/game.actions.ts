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

  gameStateChanged: (gameId: number, data: GameStateChangedData) => ({
    type: Types.GAME_STATE_CHANGED,
    gameId,
    data,
  }),

  playerJoined: (gameId: number, playerProperties: PlayerProperties) => ({
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

  playerPropertiesChanged: (gameId: number, playerId: number, properties: PlayerProperties) => ({
    type: Types.PLAYER_PROPERTIES_CHANGED,
    gameId,
    playerId,
    properties,
  }),

  kicked: (gameId: number) => ({
    type: Types.KICKED,
    gameId,
  }),

  cardMoved: (gameId: number, playerId: number, data: MoveCardData) => ({
    type: Types.CARD_MOVED,
    gameId,
    playerId,
    data,
  }),

  cardFlipped: (gameId: number, playerId: number, data: FlipCardData) => ({
    type: Types.CARD_FLIPPED,
    gameId,
    playerId,
    data,
  }),

  cardDestroyed: (gameId: number, playerId: number, data: DestroyCardData) => ({
    type: Types.CARD_DESTROYED,
    gameId,
    playerId,
    data,
  }),

  cardAttached: (gameId: number, playerId: number, data: AttachCardData) => ({
    type: Types.CARD_ATTACHED,
    gameId,
    playerId,
    data,
  }),

  tokenCreated: (gameId: number, playerId: number, data: CreateTokenData) => ({
    type: Types.TOKEN_CREATED,
    gameId,
    playerId,
    data,
  }),

  cardAttrChanged: (gameId: number, playerId: number, data: SetCardAttrData) => ({
    type: Types.CARD_ATTR_CHANGED,
    gameId,
    playerId,
    data,
  }),

  cardCounterChanged: (gameId: number, playerId: number, data: SetCardCounterData) => ({
    type: Types.CARD_COUNTER_CHANGED,
    gameId,
    playerId,
    data,
  }),

  arrowCreated: (gameId: number, playerId: number, data: CreateArrowData) => ({
    type: Types.ARROW_CREATED,
    gameId,
    playerId,
    data,
  }),

  arrowDeleted: (gameId: number, playerId: number, data: DeleteArrowData) => ({
    type: Types.ARROW_DELETED,
    gameId,
    playerId,
    data,
  }),

  counterCreated: (gameId: number, playerId: number, data: CreateCounterData) => ({
    type: Types.COUNTER_CREATED,
    gameId,
    playerId,
    data,
  }),

  counterSet: (gameId: number, playerId: number, data: SetCounterData) => ({
    type: Types.COUNTER_SET,
    gameId,
    playerId,
    data,
  }),

  counterDeleted: (gameId: number, playerId: number, data: DelCounterData) => ({
    type: Types.COUNTER_DELETED,
    gameId,
    playerId,
    data,
  }),

  cardsDrawn: (gameId: number, playerId: number, data: DrawCardsData) => ({
    type: Types.CARDS_DRAWN,
    gameId,
    playerId,
    data,
  }),

  cardsRevealed: (gameId: number, playerId: number, data: RevealCardsData) => ({
    type: Types.CARDS_REVEALED,
    gameId,
    playerId,
    data,
  }),

  zoneShuffled: (gameId: number, playerId: number, data: ShuffleData) => ({
    type: Types.ZONE_SHUFFLED,
    gameId,
    playerId,
    data,
  }),

  dieRolled: (gameId: number, playerId: number, data: RollDieData) => ({
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

  zoneDumped: (gameId: number, playerId: number, data: DumpZoneData) => ({
    type: Types.ZONE_DUMPED,
    gameId,
    playerId,
    data,
  }),

  zonePropertiesChanged: (gameId: number, playerId: number, data: ChangeZonePropertiesData) => ({
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
