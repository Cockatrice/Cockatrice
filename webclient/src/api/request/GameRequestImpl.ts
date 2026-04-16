import type { IGameRequest } from '@app/websocket';
import { GameCommands } from '@app/websocket';

import { Data } from '@app/types';

export class GameRequestImpl implements IGameRequest {
  leaveGame(gameId: number): void {
    GameCommands.leaveGame(gameId);
  }

  kickFromGame(gameId: number, params: Data.KickFromGameParams): void {
    GameCommands.kickFromGame(gameId, params);
  }

  gameSay(gameId: number, params: Data.GameSayParams): void {
    GameCommands.gameSay(gameId, params);
  }

  readyStart(gameId: number, params: Data.ReadyStartParams): void {
    GameCommands.readyStart(gameId, params);
  }

  concede(gameId: number): void {
    GameCommands.concede(gameId);
  }

  unconcede(gameId: number): void {
    GameCommands.unconcede(gameId);
  }

  judge(gameId: number, targetId: number, innerGameCommand: Data.GameCommand): void {
    GameCommands.judge(gameId, targetId, innerGameCommand);
  }

  nextTurn(gameId: number): void {
    GameCommands.nextTurn(gameId);
  }

  setActivePhase(gameId: number, params: Data.SetActivePhaseParams): void {
    GameCommands.setActivePhase(gameId, params);
  }

  reverseTurn(gameId: number): void {
    GameCommands.reverseTurn(gameId);
  }

  moveCard(gameId: number, params: Data.MoveCardParams): void {
    GameCommands.moveCard(gameId, params);
  }

  flipCard(gameId: number, params: Data.FlipCardParams): void {
    GameCommands.flipCard(gameId, params);
  }

  attachCard(gameId: number, params: Data.AttachCardParams): void {
    GameCommands.attachCard(gameId, params);
  }

  createToken(gameId: number, params: Data.CreateTokenParams): void {
    GameCommands.createToken(gameId, params);
  }

  setCardAttr(gameId: number, params: Data.SetCardAttrParams): void {
    GameCommands.setCardAttr(gameId, params);
  }

  setCardCounter(gameId: number, params: Data.SetCardCounterParams): void {
    GameCommands.setCardCounter(gameId, params);
  }

  incCardCounter(gameId: number, params: Data.IncCardCounterParams): void {
    GameCommands.incCardCounter(gameId, params);
  }

  drawCards(gameId: number, params: Data.DrawCardsParams): void {
    GameCommands.drawCards(gameId, params);
  }

  undoDraw(gameId: number): void {
    GameCommands.undoDraw(gameId);
  }

  createArrow(gameId: number, params: Data.CreateArrowParams): void {
    GameCommands.createArrow(gameId, params);
  }

  deleteArrow(gameId: number, params: Data.DeleteArrowParams): void {
    GameCommands.deleteArrow(gameId, params);
  }

  createCounter(gameId: number, params: Data.CreateCounterParams): void {
    GameCommands.createCounter(gameId, params);
  }

  setCounter(gameId: number, params: Data.SetCounterParams): void {
    GameCommands.setCounter(gameId, params);
  }

  incCounter(gameId: number, params: Data.IncCounterParams): void {
    GameCommands.incCounter(gameId, params);
  }

  delCounter(gameId: number, params: Data.DelCounterParams): void {
    GameCommands.delCounter(gameId, params);
  }

  shuffle(gameId: number, params: Data.ShuffleParams): void {
    GameCommands.shuffle(gameId, params);
  }

  dumpZone(gameId: number, params: Data.DumpZoneParams): void {
    GameCommands.dumpZone(gameId, params);
  }

  revealCards(gameId: number, params: Data.RevealCardsParams): void {
    GameCommands.revealCards(gameId, params);
  }

  changeZoneProperties(gameId: number, params: Data.ChangeZonePropertiesParams): void {
    GameCommands.changeZoneProperties(gameId, params);
  }

  deckSelect(gameId: number, params: Data.DeckSelectParams): void {
    GameCommands.deckSelect(gameId, params);
  }

  setSideboardPlan(gameId: number, params: Data.SetSideboardPlanParams): void {
    GameCommands.setSideboardPlan(gameId, params);
  }

  setSideboardLock(gameId: number, params: Data.SetSideboardLockParams): void {
    GameCommands.setSideboardLock(gameId, params);
  }

  mulligan(gameId: number, params: Data.MulliganParams): void {
    GameCommands.mulligan(gameId, params);
  }

  rollDie(gameId: number, params: Data.RollDieParams): void {
    GameCommands.rollDie(gameId, params);
  }
}
