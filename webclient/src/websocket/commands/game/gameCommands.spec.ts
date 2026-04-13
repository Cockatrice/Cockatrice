import { BackendService } from '../../services/BackendService';
import { create, setExtension } from '@bufbuild/protobuf';
import { GameCommandSchema, Command_Judge_ext } from 'generated/proto/game_commands_pb';
import { Command_DrawCardsSchema, Command_DrawCards_ext } from 'generated/proto/command_draw_cards_pb';
import { Command_AttachCard_ext } from 'generated/proto/command_attach_card_pb';
import { Command_ChangeZoneProperties_ext } from 'generated/proto/command_change_zone_properties_pb';
import { Command_Concede_ext, Command_Unconcede_ext } from 'generated/proto/command_concede_pb';
import { Command_CreateArrow_ext } from 'generated/proto/command_create_arrow_pb';
import { Command_CreateCounter_ext } from 'generated/proto/command_create_counter_pb';
import { Command_CreateToken_ext } from 'generated/proto/command_create_token_pb';
import { Command_DeckSelect_ext } from 'generated/proto/command_deck_select_pb';
import { Command_DelCounter_ext } from 'generated/proto/command_del_counter_pb';
import { Command_DeleteArrow_ext } from 'generated/proto/command_delete_arrow_pb';
import { Command_DumpZone_ext } from 'generated/proto/command_dump_zone_pb';
import { Command_FlipCard_ext } from 'generated/proto/command_flip_card_pb';
import { Command_GameSay_ext } from 'generated/proto/command_game_say_pb';
import { Command_IncCardCounter_ext } from 'generated/proto/command_inc_card_counter_pb';
import { Command_IncCounter_ext } from 'generated/proto/command_inc_counter_pb';
import { Command_KickFromGame_ext } from 'generated/proto/command_kick_from_game_pb';
import { Command_LeaveGame_ext } from 'generated/proto/command_leave_game_pb';
import { Command_MoveCard_ext } from 'generated/proto/command_move_card_pb';
import { Command_Mulligan_ext } from 'generated/proto/command_mulligan_pb';
import { Command_NextTurn_ext } from 'generated/proto/command_next_turn_pb';
import { Command_ReadyStart_ext } from 'generated/proto/command_ready_start_pb';
import { Command_RevealCards_ext } from 'generated/proto/command_reveal_cards_pb';
import { Command_ReverseTurn_ext } from 'generated/proto/command_reverse_turn_pb';
import { Command_SetActivePhase_ext } from 'generated/proto/command_set_active_phase_pb';
import { Command_SetCardAttr_ext } from 'generated/proto/command_set_card_attr_pb';
import { Command_SetCardCounter_ext } from 'generated/proto/command_set_card_counter_pb';
import { Command_SetCounter_ext } from 'generated/proto/command_set_counter_pb';
import { Command_SetSideboardLock_ext } from 'generated/proto/command_set_sideboard_lock_pb';
import { Command_SetSideboardPlan_ext } from 'generated/proto/command_set_sideboard_plan_pb';
import { Command_Shuffle_ext } from 'generated/proto/command_shuffle_pb';
import { Command_UndoDraw_ext } from 'generated/proto/command_undo_draw_pb';
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
import { unconcede } from './unconcede';
import { judge } from './judge';

vi.mock('../../services/BackendService', () => ({
  BackendService: { sendGameCommand: vi.fn() },
}));

const gameId = 1;

beforeEach(() => {
  (BackendService.sendGameCommand as vi.Mock).mockClear();
});

describe('Game commands — delegate to BackendService.sendGameCommand', () => {
  it('attachCard sends Command_AttachCard', () => {
    attachCard(gameId, { cardId: 10, startZone: 'hand' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_AttachCard_ext, expect.objectContaining({ cardId: 10, startZone: 'hand' })
    );
  });

  it('changeZoneProperties sends Command_ChangeZoneProperties', () => {
    changeZoneProperties(gameId, { zoneName: 'side' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_ChangeZoneProperties_ext, expect.objectContaining({ zoneName: 'side' })
    );
  });

  it('concede sends Command_Concede with empty object', () => {
    concede(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Concede_ext, expect.any(Object));
  });

  it('createArrow sends Command_CreateArrow', () => {
    createArrow(gameId, { startPlayerId: 1, startZone: 'hand' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateArrow_ext, expect.objectContaining({ startPlayerId: 1, startZone: 'hand' })
    );
  });

  it('createCounter sends Command_CreateCounter', () => {
    createCounter(gameId, { counterName: 'life' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateCounter_ext, expect.objectContaining({ counterName: 'life' })
    );
  });

  it('createToken sends Command_CreateToken', () => {
    createToken(gameId, { cardName: 'Goblin', zone: 'play' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_CreateToken_ext, expect.objectContaining({ cardName: 'Goblin', zone: 'play' })
    );
  });

  it('deckSelect sends Command_DeckSelect', () => {
    deckSelect(gameId, { deckId: 5 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_DeckSelect_ext, expect.objectContaining({ deckId: 5 }));
  });

  it('delCounter sends Command_DelCounter', () => {
    delCounter(gameId, { counterId: 3 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_DelCounter_ext, expect.objectContaining({ counterId: 3 }));
  });

  it('deleteArrow sends Command_DeleteArrow', () => {
    deleteArrow(gameId, { arrowId: 2 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_DeleteArrow_ext, expect.objectContaining({ arrowId: 2 }));
  });

  it('drawCards sends Command_DrawCards', () => {
    drawCards(gameId, { number: 3 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_DrawCards_ext, expect.objectContaining({ number: 3 }));
  });

  it('dumpZone sends Command_DumpZone', () => {
    dumpZone(gameId, { playerId: 2, zoneName: 'library' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_DumpZone_ext, expect.objectContaining({ playerId: 2, zoneName: 'library' })
    );
  });

  it('flipCard sends Command_FlipCard', () => {
    flipCard(gameId, { cardId: 7, faceDown: false });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_FlipCard_ext, expect.objectContaining({ cardId: 7, faceDown: false })
    );
  });

  it('gameSay sends Command_GameSay', () => {
    gameSay(gameId, { message: 'hello' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_GameSay_ext, expect.objectContaining({ message: 'hello' }));
  });

  it('incCardCounter sends Command_IncCardCounter', () => {
    incCardCounter(gameId, { cardId: 5, counterId: 1 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_IncCardCounter_ext, expect.objectContaining({ cardId: 5, counterId: 1 })
    );
  });

  it('incCounter sends Command_IncCounter', () => {
    incCounter(gameId, { counterId: 1, delta: 5 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_IncCounter_ext, expect.objectContaining({ counterId: 1, delta: 5 })
    );
  });

  it('kickFromGame sends Command_KickFromGame', () => {
    kickFromGame(gameId, { playerId: 2 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_KickFromGame_ext, expect.objectContaining({ playerId: 2 }));
  });

  it('leaveGame sends Command_LeaveGame with empty object', () => {
    leaveGame(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_LeaveGame_ext, expect.any(Object));
  });

  it('moveCard sends Command_MoveCard', () => {
    moveCard(gameId, { startZone: 'hand', targetZone: 'graveyard' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_MoveCard_ext,
      expect.objectContaining({ startZone: 'hand', targetZone: 'graveyard' })
    );
  });

  it('mulligan sends Command_Mulligan', () => {
    mulligan(gameId, { number: 7 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Mulligan_ext, expect.objectContaining({ number: 7 }));
  });

  it('nextTurn sends Command_NextTurn with empty object', () => {
    nextTurn(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_NextTurn_ext, expect.any(Object));
  });

  it('readyStart sends Command_ReadyStart', () => {
    readyStart(gameId, { ready: true });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_ReadyStart_ext, expect.objectContaining({ ready: true }));
  });

  it('revealCards sends Command_RevealCards', () => {
    revealCards(gameId, { zoneName: 'hand', cardId: [1, 2] });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_RevealCards_ext, expect.objectContaining({ zoneName: 'hand', cardId: [1, 2] })
    );
  });

  it('reverseTurn sends Command_ReverseTurn with empty object', () => {
    reverseTurn(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_ReverseTurn_ext, expect.any(Object));
  });

  it('setActivePhase sends Command_SetActivePhase', () => {
    setActivePhase(gameId, { phase: 2 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_SetActivePhase_ext, expect.objectContaining({ phase: 2 }));
  });

  it('setCardAttr sends Command_SetCardAttr', () => {
    setCardAttr(gameId, { zone: 'play', cardId: 5, attrValue: '2' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCardAttr_ext,
      expect.objectContaining({ zone: 'play', cardId: 5, attrValue: '2' })
    );
  });

  it('setCardCounter sends Command_SetCardCounter', () => {
    setCardCounter(gameId, { cardId: 5, counterId: 1 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCardCounter_ext, expect.objectContaining({ cardId: 5, counterId: 1 })
    );
  });

  it('setCounter sends Command_SetCounter', () => {
    setCounter(gameId, { counterId: 1, value: 10 });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetCounter_ext, expect.objectContaining({ counterId: 1, value: 10 })
    );
  });

  it('setSideboardLock sends Command_SetSideboardLock', () => {
    setSideboardLock(gameId, { locked: true });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetSideboardLock_ext, expect.objectContaining({ locked: true })
    );
  });

  it('setSideboardPlan sends Command_SetSideboardPlan', () => {
    setSideboardPlan(gameId, { moveList: [] });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId, Command_SetSideboardPlan_ext, expect.objectContaining({ moveList: expect.any(Array) })
    );
  });

  it('shuffle sends Command_Shuffle', () => {
    shuffle(gameId, { zoneName: 'hand' });
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Shuffle_ext, expect.objectContaining({ zoneName: 'hand' }));
  });

  it('undoDraw sends Command_UndoDraw with empty object', () => {
    undoDraw(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_UndoDraw_ext, expect.any(Object));
  });

  it('unconcede sends Command_Unconcede with empty object', () => {
    unconcede(gameId);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(gameId, Command_Unconcede_ext, expect.any(Object));
  });

  it('judge sends Command_Judge with targetId and wrapped gameCommand array', () => {
    const targetId = 3;
    const innerCmd = create(GameCommandSchema);
    setExtension(innerCmd, Command_DrawCards_ext, create(Command_DrawCardsSchema, { number: 2 }));
    judge(gameId, targetId, innerCmd);
    expect(BackendService.sendGameCommand).toHaveBeenCalledWith(
      gameId,
      Command_Judge_ext,
      expect.objectContaining({ targetId: 3, gameCommand: expect.any(Array) })
    );
  });
});
