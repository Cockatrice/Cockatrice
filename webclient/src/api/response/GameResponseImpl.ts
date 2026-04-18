import { Data } from '@app/types';
import type { IGameResponse } from '@app/websocket';
import { GameDispatch } from '@app/store';

export class GameResponseImpl implements IGameResponse {
  clearStore(): void {
    GameDispatch.clearStore();
  }

  gameStateChanged(gameId: number, data: Data.Event_GameStateChanged): void {
    GameDispatch.gameStateChanged(gameId, data);
  }

  playerJoined(gameId: number, playerProperties: Data.ServerInfo_PlayerProperties): void {
    GameDispatch.playerJoined(gameId, playerProperties);
  }

  playerLeft(gameId: number, playerId: number, reason: number): void {
    GameDispatch.playerLeft(gameId, playerId, reason);
  }

  playerPropertiesChanged(gameId: number, playerId: number, properties: Data.ServerInfo_PlayerProperties): void {
    GameDispatch.playerPropertiesChanged(gameId, playerId, properties);
  }

  gameClosed(gameId: number): void {
    GameDispatch.gameClosed(gameId);
  }

  gameHostChanged(gameId: number, hostId: number): void {
    GameDispatch.gameHostChanged(gameId, hostId);
  }

  kicked(gameId: number): void {
    GameDispatch.kicked(gameId);
  }

  gameSay(gameId: number, playerId: number, message: string, timeReceived: number): void {
    GameDispatch.gameSay(gameId, playerId, message, timeReceived);
  }

  cardMoved(gameId: number, playerId: number, data: Data.Event_MoveCard): void {
    GameDispatch.cardMoved(gameId, playerId, data);
  }

  cardFlipped(gameId: number, playerId: number, data: Data.Event_FlipCard): void {
    GameDispatch.cardFlipped(gameId, playerId, data);
  }

  cardDestroyed(gameId: number, playerId: number, data: Data.Event_DestroyCard): void {
    GameDispatch.cardDestroyed(gameId, playerId, data);
  }

  cardAttached(gameId: number, playerId: number, data: Data.Event_AttachCard): void {
    GameDispatch.cardAttached(gameId, playerId, data);
  }

  tokenCreated(gameId: number, playerId: number, data: Data.Event_CreateToken): void {
    GameDispatch.tokenCreated(gameId, playerId, data);
  }

  cardAttrChanged(gameId: number, playerId: number, data: Data.Event_SetCardAttr): void {
    GameDispatch.cardAttrChanged(gameId, playerId, data);
  }

  cardCounterChanged(gameId: number, playerId: number, data: Data.Event_SetCardCounter): void {
    GameDispatch.cardCounterChanged(gameId, playerId, data);
  }

  arrowCreated(gameId: number, playerId: number, data: Data.Event_CreateArrow): void {
    GameDispatch.arrowCreated(gameId, playerId, data);
  }

  arrowDeleted(gameId: number, playerId: number, data: Data.Event_DeleteArrow): void {
    GameDispatch.arrowDeleted(gameId, playerId, data);
  }

  counterCreated(gameId: number, playerId: number, data: Data.Event_CreateCounter): void {
    GameDispatch.counterCreated(gameId, playerId, data);
  }

  counterSet(gameId: number, playerId: number, data: Data.Event_SetCounter): void {
    GameDispatch.counterSet(gameId, playerId, data);
  }

  counterDeleted(gameId: number, playerId: number, data: Data.Event_DelCounter): void {
    GameDispatch.counterDeleted(gameId, playerId, data);
  }

  cardsDrawn(gameId: number, playerId: number, data: Data.Event_DrawCards): void {
    GameDispatch.cardsDrawn(gameId, playerId, data);
  }

  cardsRevealed(gameId: number, playerId: number, data: Data.Event_RevealCards): void {
    GameDispatch.cardsRevealed(gameId, playerId, data);
  }

  zoneShuffled(gameId: number, playerId: number, data: Data.Event_Shuffle): void {
    GameDispatch.zoneShuffled(gameId, playerId, data);
  }

  dieRolled(gameId: number, playerId: number, data: Data.Event_RollDie): void {
    GameDispatch.dieRolled(gameId, playerId, data);
  }

  activePlayerSet(gameId: number, activePlayerId: number): void {
    GameDispatch.activePlayerSet(gameId, activePlayerId);
  }

  activePhaseSet(gameId: number, phase: number): void {
    GameDispatch.activePhaseSet(gameId, phase);
  }

  turnReversed(gameId: number, reversed: boolean): void {
    GameDispatch.turnReversed(gameId, reversed);
  }

  zoneDumped(gameId: number, playerId: number, data: Data.Event_DumpZone): void {
    GameDispatch.zoneDumped(gameId, playerId, data);
  }

  zonePropertiesChanged(gameId: number, playerId: number, data: Data.Event_ChangeZoneProperties): void {
    GameDispatch.zonePropertiesChanged(gameId, playerId, data);
  }
}
