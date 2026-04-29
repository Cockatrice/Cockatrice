const { mockDispatch } = vi.hoisted(() => ({ mockDispatch: vi.fn() }));
vi.mock('../store', () => ({ store: { dispatch: mockDispatch } }));

import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';
import { Actions } from './game.actions';
import { Dispatch } from './game.dispatch';
import {
  makeArrow,
  makeCard,
  makeCounter,
  makePlayerProperties,
} from './__mocks__/fixtures';

beforeEach(() => {
  mockDispatch.mockClear();
});

describe('Dispatch', () => {
  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(mockDispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('gameJoined dispatches Actions.gameJoined()', () => {
    const data = create(Data.Event_GameJoinedSchema, { hostId: 1, playerId: 2 });
    Dispatch.gameJoined(data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameJoined({ data }));
  });

  it('gameLeft dispatches Actions.gameLeft()', () => {
    Dispatch.gameLeft(2);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameLeft({ gameId: 2 }));
  });

  it('gameClosed dispatches Actions.gameClosed()', () => {
    Dispatch.gameClosed(3);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameClosed({ gameId: 3 }));
  });

  it('gameHostChanged dispatches Actions.gameHostChanged()', () => {
    Dispatch.gameHostChanged(1, 7);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameHostChanged({ gameId: 1, hostId: 7 }));
  });

  it('gameStateChanged dispatches Actions.gameStateChanged()', () => {
    const data = create(Data.Event_GameStateChangedSchema, {
      playerList: [], gameStarted: false, activePlayerId: 0, activePhase: 0, secondsElapsed: 0
    });
    Dispatch.gameStateChanged(1, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameStateChanged({ gameId: 1, data }));
  });

  it('playerJoined dispatches Actions.playerJoined()', () => {
    const props = makePlayerProperties();
    Dispatch.playerJoined(1, props);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.playerJoined({ gameId: 1, playerProperties: props }));
  });

  it('playerLeft dispatches Actions.playerLeft() with reason + timeReceived', () => {
    const nowSpy = vi.spyOn(Date, 'now').mockReturnValue(9999);
    Dispatch.playerLeft(1, 2, 3);
    expect(mockDispatch).toHaveBeenCalledWith(
      Actions.playerLeft({ gameId: 1, playerId: 2, reason: 3, timeReceived: 9999 }),
    );
    nowSpy.mockRestore();
  });

  it('playerPropertiesChanged dispatches Actions.playerPropertiesChanged()', () => {
    const props = makePlayerProperties();
    Dispatch.playerPropertiesChanged(1, 2, props);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.playerPropertiesChanged({ gameId: 1, playerId: 2, properties: props }));
  });

  it('kicked dispatches Actions.kicked()', () => {
    Dispatch.kicked(1);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.kicked({ gameId: 1 }));
  });

  it('cardMoved dispatches Actions.cardMoved()', () => {
    const data = create(Data.Event_MoveCardSchema, { cardId: 1 });
    Dispatch.cardMoved(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardMoved({ gameId: 1, playerId: 2, data }));
  });

  it('cardFlipped dispatches Actions.cardFlipped()', () => {
    const data = create(Data.Event_FlipCardSchema, { cardId: 1 });
    Dispatch.cardFlipped(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardFlipped({ gameId: 1, playerId: 2, data }));
  });

  it('cardDestroyed dispatches Actions.cardDestroyed()', () => {
    const data = create(Data.Event_DestroyCardSchema, { cardId: 1 });
    Dispatch.cardDestroyed(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardDestroyed({ gameId: 1, playerId: 2, data }));
  });

  it('cardAttached dispatches Actions.cardAttached()', () => {
    const data = create(Data.Event_AttachCardSchema, { cardId: 1 });
    Dispatch.cardAttached(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardAttached({ gameId: 1, playerId: 2, data }));
  });

  it('tokenCreated dispatches Actions.tokenCreated()', () => {
    const data = create(Data.Event_CreateTokenSchema, { cardId: 1 });
    Dispatch.tokenCreated(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.tokenCreated({ gameId: 1, playerId: 2, data }));
  });

  it('cardAttrChanged dispatches Actions.cardAttrChanged()', () => {
    const data = create(Data.Event_SetCardAttrSchema, { cardId: 1 });
    Dispatch.cardAttrChanged(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardAttrChanged({ gameId: 1, playerId: 2, data }));
  });

  it('cardCounterChanged dispatches Actions.cardCounterChanged()', () => {
    const data = create(Data.Event_SetCardCounterSchema, { cardId: 1 });
    Dispatch.cardCounterChanged(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardCounterChanged({ gameId: 1, playerId: 2, data }));
  });

  it('arrowCreated dispatches Actions.arrowCreated()', () => {
    const data = create(Data.Event_CreateArrowSchema, { arrowInfo: makeArrow() });
    Dispatch.arrowCreated(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.arrowCreated({ gameId: 1, playerId: 2, data }));
  });

  it('arrowDeleted dispatches Actions.arrowDeleted()', () => {
    const data = create(Data.Event_DeleteArrowSchema, { arrowId: 3 });
    Dispatch.arrowDeleted(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.arrowDeleted({ gameId: 1, playerId: 2, data }));
  });

  it('counterCreated dispatches Actions.counterCreated()', () => {
    const data = create(Data.Event_CreateCounterSchema, { counterInfo: makeCounter() });
    Dispatch.counterCreated(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.counterCreated({ gameId: 1, playerId: 2, data }));
  });

  it('counterSet dispatches Actions.counterSet()', () => {
    const data = create(Data.Event_SetCounterSchema, { counterId: 1, value: 10 });
    Dispatch.counterSet(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.counterSet({ gameId: 1, playerId: 2, data }));
  });

  it('counterDeleted dispatches Actions.counterDeleted()', () => {
    const data = create(Data.Event_DelCounterSchema, { counterId: 1 });
    Dispatch.counterDeleted(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.counterDeleted({ gameId: 1, playerId: 2, data }));
  });

  it('cardsDrawn dispatches Actions.cardsDrawn()', () => {
    const data = create(Data.Event_DrawCardsSchema, { number: 2, cards: [makeCard()] });
    Dispatch.cardsDrawn(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardsDrawn({ gameId: 1, playerId: 2, data }));
  });

  it('cardsRevealed dispatches Actions.cardsRevealed()', () => {
    const data = create(Data.Event_RevealCardsSchema, { zoneName: 'hand', cards: [] });
    Dispatch.cardsRevealed(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.cardsRevealed({ gameId: 1, playerId: 2, data }));
  });

  it('zoneShuffled dispatches Actions.zoneShuffled()', () => {
    const data = create(Data.Event_ShuffleSchema, { zoneName: 'deck', start: 0, end: 39 });
    Dispatch.zoneShuffled(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.zoneShuffled({ gameId: 1, playerId: 2, data }));
  });

  it('dieRolled dispatches Actions.dieRolled()', () => {
    const data = create(Data.Event_RollDieSchema, { sides: 6, value: 4, values: [4] });
    Dispatch.dieRolled(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.dieRolled({ gameId: 1, playerId: 2, data }));
  });

  it('activePlayerSet dispatches Actions.activePlayerSet()', () => {
    Dispatch.activePlayerSet(1, 3);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.activePlayerSet({ gameId: 1, activePlayerId: 3 }));
  });

  it('activePhaseSet dispatches Actions.activePhaseSet()', () => {
    Dispatch.activePhaseSet(1, 2);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.activePhaseSet({ gameId: 1, phase: 2 }));
  });

  it('turnReversed dispatches Actions.turnReversed()', () => {
    Dispatch.turnReversed(1, true);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.turnReversed({ gameId: 1, reversed: true }));
  });

  it('zoneDumped dispatches Actions.zoneDumped()', () => {
    const data = create(Data.Event_DumpZoneSchema, { zoneOwnerId: 1, zoneName: 'hand', numberCards: 3, isReversed: false });
    Dispatch.zoneDumped(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.zoneDumped({ gameId: 1, playerId: 2, data }));
  });

  it('zonePropertiesChanged dispatches Actions.zonePropertiesChanged()', () => {
    const data = create(Data.Event_ChangeZonePropertiesSchema, { zoneName: 'deck', alwaysRevealTopCard: true, alwaysLookAtTopCard: false });
    Dispatch.zonePropertiesChanged(1, 2, data);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.zonePropertiesChanged({ gameId: 1, playerId: 2, data }));
  });

  it('gameSay dispatches Actions.gameSay()', () => {
    Dispatch.gameSay(1, 2, 'gg wp', 1700000000000);
    expect(mockDispatch).toHaveBeenCalledWith(Actions.gameSay({ gameId: 1, playerId: 2, message: 'gg wp', timeReceived: 1700000000000 }));
  });
});
