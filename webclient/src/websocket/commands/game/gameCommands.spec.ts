vi.mock('../../WebClient');

import { WebClient } from '../../WebClient';
import { create, setExtension } from '@bufbuild/protobuf';
import {
  Command_AttachCard_ext,
  Command_ChangeZoneProperties_ext,
  Command_Concede_ext,
  Command_CreateArrow_ext,
  Command_CreateCounter_ext,
  Command_CreateToken_ext,
  Command_DeckSelect_ext,
  Command_DelCounter_ext,
  Command_DeleteArrow_ext,
  Command_DrawCards_ext,
  Command_DrawCardsSchema,
  Command_DumpZone_ext,
  Command_FlipCard_ext,
  Command_GameSay_ext,
  Command_IncCardCounter_ext,
  Command_IncCounter_ext,
  Command_Judge_ext,
  Command_KickFromGame_ext,
  Command_LeaveGame_ext,
  Command_MoveCard_ext,
  Command_Mulligan_ext,
  Command_NextTurn_ext,
  Command_ReadyStart_ext,
  Command_RevealCards_ext,
  Command_ReverseTurn_ext,
  Command_RollDie_ext,
  Command_SetActivePhase_ext,
  Command_SetCardAttr_ext,
  Command_SetCardCounter_ext,
  Command_SetCounter_ext,
  Command_SetSideboardLock_ext,
  Command_SetSideboardPlan_ext,
  Command_Shuffle_ext,
  Command_UndoDraw_ext,
  Command_Unconcede_ext,
  GameCommandSchema,
} from '@app/generated';

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
import { rollDie } from './rollDie';
import { setActivePhase } from './setActivePhase';
import { setCardAttr } from './setCardAttr';
import { setCardCounter } from './setCardCounter';
import { setCounter } from './setCounter';
import { setSideboardLock } from './setSideboardLock';
import { setSideboardPlan } from './setSideboardPlan';
import { shuffle } from './shuffle';
import { undoDraw } from './undoDraw';
import { unconcede } from './unconcede';
import { judge } from './judge';

const gameId = 1;

describe('Game commands — delegate to WebClient.instance.protobuf.sendGameCommand', () => {
  it('attachCard sends Command_AttachCard', () => {
    attachCard(gameId, { cardId: 10, startZone: 'hand' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_AttachCard_ext, expect.objectContaining({ cardId: 10, startZone: 'hand' })
    );
  });

  it('changeZoneProperties sends Command_ChangeZoneProperties', () => {
    changeZoneProperties(gameId, { zoneName: 'side' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_ChangeZoneProperties_ext, expect.objectContaining({ zoneName: 'side' })
    );
  });

  it('concede sends Command_Concede with empty object', () => {
    concede(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Concede_ext, expect.any(Object));
  });

  it('createArrow sends Command_CreateArrow', () => {
    createArrow(gameId, { startPlayerId: 1, startZone: 'hand' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateArrow_ext, expect.objectContaining({ startPlayerId: 1, startZone: 'hand' })
    );
  });

  it('createCounter sends Command_CreateCounter', () => {
    createCounter(gameId, { counterName: 'life' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateCounter_ext, expect.objectContaining({ counterName: 'life' })
    );
  });

  it('createToken sends Command_CreateToken', () => {
    createToken(gameId, { cardName: 'Goblin', zone: 'play' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateToken_ext, expect.objectContaining({ cardName: 'Goblin', zone: 'play' })
    );
  });

  it('deckSelect sends Command_DeckSelect', () => {
    deckSelect(gameId, { deckId: 5 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DeckSelect_ext, expect.objectContaining({ deckId: 5 })
    );
  });

  it('delCounter sends Command_DelCounter', () => {
    delCounter(gameId, { counterId: 3 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DelCounter_ext, expect.objectContaining({ counterId: 3 })
    );
  });

  it('deleteArrow sends Command_DeleteArrow', () => {
    deleteArrow(gameId, { arrowId: 2 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DeleteArrow_ext, expect.objectContaining({ arrowId: 2 })
    );
  });

  it('drawCards sends Command_DrawCards', () => {
    drawCards(gameId, { number: 3 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DrawCards_ext, expect.objectContaining({ number: 3 })
    );
  });

  it('dumpZone sends Command_DumpZone', () => {
    dumpZone(gameId, { playerId: 2, zoneName: 'library' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DumpZone_ext, expect.objectContaining({ playerId: 2, zoneName: 'library' })
    );
  });

  it('flipCard sends Command_FlipCard', () => {
    flipCard(gameId, { cardId: 7, faceDown: false });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_FlipCard_ext, expect.objectContaining({ cardId: 7, faceDown: false })
    );
  });

  it('gameSay sends Command_GameSay', () => {
    gameSay(gameId, { message: 'hello' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_GameSay_ext, expect.objectContaining({ message: 'hello' })
    );
  });

  it('incCardCounter sends Command_IncCardCounter', () => {
    incCardCounter(gameId, { cardId: 5, counterId: 1 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_IncCardCounter_ext, expect.objectContaining({ cardId: 5, counterId: 1 })
    );
  });

  it('incCounter sends Command_IncCounter', () => {
    incCounter(gameId, { counterId: 1, delta: 5 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_IncCounter_ext, expect.objectContaining({ counterId: 1, delta: 5 })
    );
  });

  it('kickFromGame sends Command_KickFromGame', () => {
    kickFromGame(gameId, { playerId: 2 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_KickFromGame_ext, expect.objectContaining({ playerId: 2 })
    );
  });

  it('leaveGame sends Command_LeaveGame with empty object', () => {
    leaveGame(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_LeaveGame_ext, expect.any(Object));
  });

  it('moveCard sends Command_MoveCard', () => {
    moveCard(gameId, { startZone: 'hand', targetZone: 'graveyard' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_MoveCard_ext,
      expect.objectContaining({ startZone: 'hand', targetZone: 'graveyard' })
    );
  });

  it('mulligan sends Command_Mulligan', () => {
    mulligan(gameId, { number: 7 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_Mulligan_ext, expect.objectContaining({ number: 7 })
    );
  });

  it('nextTurn sends Command_NextTurn with empty object', () => {
    nextTurn(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_NextTurn_ext, expect.any(Object));
  });

  it('readyStart sends Command_ReadyStart', () => {
    readyStart(gameId, { ready: true });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_ReadyStart_ext, expect.objectContaining({ ready: true })
    );
  });

  it('revealCards sends Command_RevealCards', () => {
    revealCards(gameId, { zoneName: 'hand', cardId: [1, 2] });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_RevealCards_ext, expect.objectContaining({ zoneName: 'hand', cardId: [1, 2] })
    );
  });

  it('reverseTurn sends Command_ReverseTurn with empty object', () => {
    reverseTurn(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_ReverseTurn_ext, expect.any(Object));
  });

  it('setActivePhase sends Command_SetActivePhase', () => {
    setActivePhase(gameId, { phase: 2 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetActivePhase_ext, expect.objectContaining({ phase: 2 })
    );
  });

  it('setCardAttr sends Command_SetCardAttr', () => {
    setCardAttr(gameId, { zone: 'play', cardId: 5, attrValue: '2' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCardAttr_ext,
      expect.objectContaining({ zone: 'play', cardId: 5, attrValue: '2' })
    );
  });

  it('setCardCounter sends Command_SetCardCounter', () => {
    setCardCounter(gameId, { cardId: 5, counterId: 1 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCardCounter_ext, expect.objectContaining({ cardId: 5, counterId: 1 })
    );
  });

  it('setCounter sends Command_SetCounter', () => {
    setCounter(gameId, { counterId: 1, value: 10 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCounter_ext, expect.objectContaining({ counterId: 1, value: 10 })
    );
  });

  it('setSideboardLock sends Command_SetSideboardLock', () => {
    setSideboardLock(gameId, { locked: true });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetSideboardLock_ext, expect.objectContaining({ locked: true })
    );
  });

  it('setSideboardPlan sends Command_SetSideboardPlan', () => {
    setSideboardPlan(gameId, { moveList: [] });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetSideboardPlan_ext, expect.objectContaining({ moveList: expect.any(Array) })
    );
  });

  it('shuffle sends Command_Shuffle', () => {
    shuffle(gameId, { zoneName: 'hand' });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_Shuffle_ext, expect.objectContaining({ zoneName: 'hand' })
    );
  });

  it('undoDraw sends Command_UndoDraw with empty object', () => {
    undoDraw(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_UndoDraw_ext, expect.any(Object));
  });

  it('unconcede sends Command_Unconcede with empty object', () => {
    unconcede(gameId);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Unconcede_ext, expect.any(Object));
  });

  it('rollDie sends Command_RollDie', () => {
    rollDie(gameId, { sides: 6, count: 2 });
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_RollDie_ext, expect.objectContaining({ sides: 6, count: 2 })
    );
  });

  it('judge sends Command_Judge with targetId and wrapped gameCommand array', () => {
    const targetId = 3;
    const innerCmd = create(GameCommandSchema);
    setExtension(innerCmd, Command_DrawCards_ext, create(Command_DrawCardsSchema, { number: 2 }));
    judge(gameId, targetId, innerCmd);
    expect(WebClient.instance.protobuf.sendGameCommand).toHaveBeenCalledWith(
      gameId,
      Command_Judge_ext,
      expect.objectContaining({ targetId: 3, gameCommand: expect.any(Array) })
    );
  });
});
