import { GameDispatch } from '@app/store';
import { Data } from '@app/types';

export class GamePersistence {
  static gameStateChanged(gameId: number, data: Data.Event_GameStateChanged): void {
    GameDispatch.gameStateChanged(gameId, data);
  }

  static playerJoined(gameId: number, playerProperties: Data.ServerInfo_PlayerProperties): void {
    GameDispatch.playerJoined(gameId, playerProperties);
  }

  static playerLeft(gameId: number, playerId: number, reason: number): void {
    GameDispatch.playerLeft(gameId, playerId, reason);
  }

  static playerPropertiesChanged(gameId: number, playerId: number, properties: Data.ServerInfo_PlayerProperties): void {
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

  static cardMoved(gameId: number, playerId: number, data: Data.Event_MoveCard): void {
    GameDispatch.cardMoved(gameId, playerId, data);
  }

  static cardFlipped(gameId: number, playerId: number, data: Data.Event_FlipCard): void {
    GameDispatch.cardFlipped(gameId, playerId, data);
  }

  static cardDestroyed(gameId: number, playerId: number, data: Data.Event_DestroyCard): void {
    GameDispatch.cardDestroyed(gameId, playerId, data);
  }

  static cardAttached(gameId: number, playerId: number, data: Data.Event_AttachCard): void {
    GameDispatch.cardAttached(gameId, playerId, data);
  }

  static tokenCreated(gameId: number, playerId: number, data: Data.Event_CreateToken): void {
    GameDispatch.tokenCreated(gameId, playerId, data);
  }

  static cardAttrChanged(gameId: number, playerId: number, data: Data.Event_SetCardAttr): void {
    GameDispatch.cardAttrChanged(gameId, playerId, data);
  }

  static cardCounterChanged(gameId: number, playerId: number, data: Data.Event_SetCardCounter): void {
    GameDispatch.cardCounterChanged(gameId, playerId, data);
  }

  static arrowCreated(gameId: number, playerId: number, data: Data.Event_CreateArrow): void {
    GameDispatch.arrowCreated(gameId, playerId, data);
  }

  static arrowDeleted(gameId: number, playerId: number, data: Data.Event_DeleteArrow): void {
    GameDispatch.arrowDeleted(gameId, playerId, data);
  }

  static counterCreated(gameId: number, playerId: number, data: Data.Event_CreateCounter): void {
    GameDispatch.counterCreated(gameId, playerId, data);
  }

  static counterSet(gameId: number, playerId: number, data: Data.Event_SetCounter): void {
    GameDispatch.counterSet(gameId, playerId, data);
  }

  static counterDeleted(gameId: number, playerId: number, data: Data.Event_DelCounter): void {
    GameDispatch.counterDeleted(gameId, playerId, data);
  }

  static cardsDrawn(gameId: number, playerId: number, data: Data.Event_DrawCards): void {
    GameDispatch.cardsDrawn(gameId, playerId, data);
  }

  static cardsRevealed(gameId: number, playerId: number, data: Data.Event_RevealCards): void {
    GameDispatch.cardsRevealed(gameId, playerId, data);
  }

  static zoneShuffled(gameId: number, playerId: number, data: Data.Event_Shuffle): void {
    GameDispatch.zoneShuffled(gameId, playerId, data);
  }

  static dieRolled(gameId: number, playerId: number, data: Data.Event_RollDie): void {
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

  static zoneDumped(gameId: number, playerId: number, data: Data.Event_DumpZone): void {
    GameDispatch.zoneDumped(gameId, playerId, data);
  }

  static zonePropertiesChanged(gameId: number, playerId: number, data: Data.Event_ChangeZoneProperties): void {
    GameDispatch.zonePropertiesChanged(gameId, playerId, data);
  }
}

