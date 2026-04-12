import { BackendService } from '../../services/BackendService';
import { attachCard } from './attachCard';
import { changeZoneProperties } from './changeZoneProperties';
import { concede } from './concede';
import { createArrow } from './createArrow';
import { createCounter } from './createCounter';
import { createToken } from './createToken';
import { deckSelect } from './deckSelect';
import { delCounter } from './delCounter';
import { deleteArrow } from './deleteArrow';
import { drawCards } from './drawCards';
import { dumpZone } from './dumpZone';
import { flipCard } from './flipCard';
import { gameSay } from './gameSay';
import { incCardCounter } from './incCardCounter';
import { incCounter } from './incCounter';
import { kickFromGame } from './kickFromGame';
import { leaveGame } from './leaveGame';
import { moveCard } from './moveCard';
import { mulligan } from './mulligan';
import { nextTurn } from './nextTurn';
import { readyStart } from './readyStart';
import { revealCards } from './revealCards';
import { reverseTurn } from './reverseTurn';
import { setActivePhase } from './setActivePhase';
import { setCardAttr } from './setCardAttr';
import { setCardCounter } from './setCardCounter';
import { setCounter } from './setCounter';
import { setSideboardLock } from './setSideboardLock';
import { setSideboardPlan } from './setSideboardPlan';
import { shuffle } from './shuffle';
import { undoDraw } from './undoDraw';

jest.mock('../../services/BackendService', () => ({
  BackendService: { sendGameCommand: jest.fn() },
}));

const gameId = 1;
const params = {} as any;

beforeEach(() => {
  (BackendService.sendGameCommand as jest.Mock).mockClear();
});

describe('Game commands — delegate to BackendService.sendGameCommand', () => {
  it('attachCard sends Command_AttachCard', () => {
    attachCard(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_AttachCard', params);
  });

  it('changeZoneProperties sends Command_ChangeZoneProperties', () => {
    changeZoneProperties(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_ChangeZoneProperties', params);
  });

  it('concede sends Command_Concede with empty object', () => {
    concede(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_Concede', {});
  });

  it('createArrow sends Command_CreateArrow', () => {
    createArrow(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_CreateArrow', params);
  });

  it('createCounter sends Command_CreateCounter', () => {
    createCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_CreateCounter', params);
  });

  it('createToken sends Command_CreateToken', () => {
    createToken(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_CreateToken', params);
  });

  it('deckSelect sends Command_DeckSelect', () => {
    deckSelect(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_DeckSelect', params);
  });

  it('delCounter sends Command_DelCounter', () => {
    delCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_DelCounter', params);
  });

  it('deleteArrow sends Command_DeleteArrow', () => {
    deleteArrow(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_DeleteArrow', params);
  });

  it('drawCards sends Command_DrawCards', () => {
    drawCards(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_DrawCards', params);
  });

  it('dumpZone sends Command_DumpZone', () => {
    dumpZone(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_DumpZone', params);
  });

  it('flipCard sends Command_FlipCard', () => {
    flipCard(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_FlipCard', params);
  });

  it('gameSay sends Command_GameSay', () => {
    gameSay(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_GameSay', params);
  });

  it('incCardCounter sends Command_IncCardCounter', () => {
    incCardCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_IncCardCounter', params);
  });

  it('incCounter sends Command_IncCounter', () => {
    incCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_IncCounter', params);
  });

  it('kickFromGame sends Command_KickFromGame', () => {
    kickFromGame(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_KickFromGame', params);
  });

  it('leaveGame sends Command_LeaveGame with empty object', () => {
    leaveGame(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_LeaveGame', {});
  });

  it('moveCard sends Command_MoveCard', () => {
    moveCard(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_MoveCard', params);
  });

  it('mulligan sends Command_Mulligan', () => {
    mulligan(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_Mulligan', params);
  });

  it('nextTurn sends Command_NextTurn with empty object', () => {
    nextTurn(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_NextTurn', {});
  });

  it('readyStart sends Command_ReadyStart', () => {
    readyStart(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_ReadyStart', params);
  });

  it('revealCards sends Command_RevealCards', () => {
    revealCards(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_RevealCards', params);
  });

  it('reverseTurn sends Command_ReverseTurn with empty object', () => {
    reverseTurn(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_ReverseTurn', {});
  });

  it('setActivePhase sends Command_SetActivePhase', () => {
    setActivePhase(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetActivePhase', params);
  });

  it('setCardAttr sends Command_SetCardAttr', () => {
    setCardAttr(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetCardAttr', params);
  });

  it('setCardCounter sends Command_SetCardCounter', () => {
    setCardCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetCardCounter', params);
  });

  it('setCounter sends Command_SetCounter', () => {
    setCounter(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetCounter', params);
  });

  it('setSideboardLock sends Command_SetSideboardLock', () => {
    setSideboardLock(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetSideboardLock', params);
  });

  it('setSideboardPlan sends Command_SetSideboardPlan', () => {
    setSideboardPlan(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_SetSideboardPlan', params);
  });

  it('shuffle sends Command_Shuffle', () => {
    shuffle(gameId, params);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_Shuffle', params);
  });

  it('undoDraw sends Command_UndoDraw with empty object', () => {
    undoDraw(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, 'Command_UndoDraw', {});
  });
});
