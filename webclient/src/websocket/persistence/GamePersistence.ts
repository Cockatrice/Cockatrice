import { GameDispatch } from 'store';
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

export class GamePersistence {
  static gameStateChanged(gameId: number, data: GameStateChangedData): void {
    GameDispatch.gameStateChanged(gameId, data);
  }

  static playerJoined(gameId: number, playerProperties: PlayerProperties): void {
    GameDispatch.playerJoined(gameId, playerProperties);
  }

  static playerLeft(gameId: number, playerId: number, reason: number): void {
    GameDispatch.playerLeft(gameId, playerId, reason);
  }

  static playerPropertiesChanged(gameId: number, playerId: number, properties: PlayerProperties): void {
    GameDispatch.playerPropertiesChanged(gameId, playerId, properties);
  }

  static gameClosed(gameId: number): void {
    GameDispatch.gameClosed(gameId);
  }

  static gameHostChanged(gameId: number, hostId: number): void {
    GameDispatch.gameHostChanged(gameId, hostId);
  }

  static kicked(gameId: number): void {
    GameDispatch.kicked(gameId);
  }

  static gameSay(gameId: number, playerId: number, message: string): void {
    GameDispatch.gameSay(gameId, playerId, message);
  }

  static cardMoved(gameId: number, playerId: number, data: MoveCardData): void {
    GameDispatch.cardMoved(gameId, playerId, data);
  }

  static cardFlipped(gameId: number, playerId: number, data: FlipCardData): void {
    GameDispatch.cardFlipped(gameId, playerId, data);
  }

  static cardDestroyed(gameId: number, playerId: number, data: DestroyCardData): void {
    GameDispatch.cardDestroyed(gameId, playerId, data);
  }

  static cardAttached(gameId: number, playerId: number, data: AttachCardData): void {
    GameDispatch.cardAttached(gameId, playerId, data);
  }

  static tokenCreated(gameId: number, playerId: number, data: CreateTokenData): void {
    GameDispatch.tokenCreated(gameId, playerId, data);
  }

  static cardAttrChanged(gameId: number, playerId: number, data: SetCardAttrData): void {
    GameDispatch.cardAttrChanged(gameId, playerId, data);
  }

  static cardCounterChanged(gameId: number, playerId: number, data: SetCardCounterData): void {
    GameDispatch.cardCounterChanged(gameId, playerId, data);
  }

  static arrowCreated(gameId: number, playerId: number, data: CreateArrowData): void {
    GameDispatch.arrowCreated(gameId, playerId, data);
  }

  static arrowDeleted(gameId: number, playerId: number, data: DeleteArrowData): void {
    GameDispatch.arrowDeleted(gameId, playerId, data);
  }

  static counterCreated(gameId: number, playerId: number, data: CreateCounterData): void {
    GameDispatch.counterCreated(gameId, playerId, data);
  }

  static counterSet(gameId: number, playerId: number, data: SetCounterData): void {
    GameDispatch.counterSet(gameId, playerId, data);
  }

  static counterDeleted(gameId: number, playerId: number, data: DelCounterData): void {
    GameDispatch.counterDeleted(gameId, playerId, data);
  }

  static cardsDrawn(gameId: number, playerId: number, data: DrawCardsData): void {
    GameDispatch.cardsDrawn(gameId, playerId, data);
  }

  static cardsRevealed(gameId: number, playerId: number, data: RevealCardsData): void {
    GameDispatch.cardsRevealed(gameId, playerId, data);
  }

  static zoneShuffled(gameId: number, playerId: number, data: ShuffleData): void {
    GameDispatch.zoneShuffled(gameId, playerId, data);
  }

  static dieRolled(gameId: number, playerId: number, data: RollDieData): void {
    GameDispatch.dieRolled(gameId, playerId, data);
  }

  static activePlayerSet(gameId: number, activePlayerId: number): void {
    GameDispatch.activePlayerSet(gameId, activePlayerId);
  }

  static activePhaseSet(gameId: number, phase: number): void {
    GameDispatch.activePhaseSet(gameId, phase);
  }

  static turnReversed(gameId: number, reversed: boolean): void {
    GameDispatch.turnReversed(gameId, reversed);
  }

  static zoneDumped(gameId: number, playerId: number, data: DumpZoneData): void {
    GameDispatch.zoneDumped(gameId, playerId, data);
  }

  static zonePropertiesChanged(gameId: number, playerId: number, data: ChangeZonePropertiesData): void {
    GameDispatch.zonePropertiesChanged(gameId, playerId, data);
  }
}

