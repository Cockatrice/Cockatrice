vi.mock('store/store', () => ({ store: { dispatch: vi.fn() } }));

import { store } from 'store/store';
import { Actions } from './game.actions';
import { Dispatch } from './game.dispatch';
import {
  makeArrow,
  makeCard,
  makeCounter,
  makeGameEntry,
  makePlayerProperties,
} from './__mocks__/fixtures';

beforeEach(() => vi.clearAllMocks());

describe('Dispatch', () => {
  it('clearStore dispatches Actions.clearStore()', () => {
    Dispatch.clearStore();
    expect(store.dispatch).toHaveBeenCalledWith(Actions.clearStore());
  });

  it('gameJoined dispatches Actions.gameJoined()', () => {
    const entry = makeGameEntry();
    Dispatch.gameJoined(1, entry);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameJoined(1, entry));
  });

  it('gameLeft dispatches Actions.gameLeft()', () => {
    Dispatch.gameLeft(2);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameLeft(2));
  });

  it('gameClosed dispatches Actions.gameClosed()', () => {
    Dispatch.gameClosed(3);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameClosed(3));
  });

  it('gameHostChanged dispatches Actions.gameHostChanged()', () => {
    Dispatch.gameHostChanged(1, 7);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameHostChanged(1, 7));
  });

  it('gameStateChanged dispatches Actions.gameStateChanged()', () => {
    const data = { playerList: [], gameStarted: false, activePlayerId: 0, activePhase: 0, secondsElapsed: 0 };
    Dispatch.gameStateChanged(1, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameStateChanged(1, data));
  });

  it('playerJoined dispatches Actions.playerJoined()', () => {
    const props = makePlayerProperties();
    Dispatch.playerJoined(1, props);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.playerJoined(1, props));
  });

  it('playerLeft dispatches Actions.playerLeft()', () => {
    Dispatch.playerLeft(1, 2, 3);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.playerLeft(1, 2, 3));
  });

  it('playerPropertiesChanged dispatches Actions.playerPropertiesChanged()', () => {
    const props = makePlayerProperties();
    Dispatch.playerPropertiesChanged(1, 2, props);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.playerPropertiesChanged(1, 2, props));
  });

  it('kicked dispatches Actions.kicked()', () => {
    Dispatch.kicked(1);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.kicked(1));
  });

  it('cardMoved dispatches Actions.cardMoved()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardMoved(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardMoved(1, 2, data));
  });

  it('cardFlipped dispatches Actions.cardFlipped()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardFlipped(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardFlipped(1, 2, data));
  });

  it('cardDestroyed dispatches Actions.cardDestroyed()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardDestroyed(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardDestroyed(1, 2, data));
  });

  it('cardAttached dispatches Actions.cardAttached()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardAttached(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardAttached(1, 2, data));
  });

  it('tokenCreated dispatches Actions.tokenCreated()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.tokenCreated(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.tokenCreated(1, 2, data));
  });

  it('cardAttrChanged dispatches Actions.cardAttrChanged()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardAttrChanged(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardAttrChanged(1, 2, data));
  });

  it('cardCounterChanged dispatches Actions.cardCounterChanged()', () => {
    const data = { cardId: 1 } as any;
    Dispatch.cardCounterChanged(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardCounterChanged(1, 2, data));
  });

  it('arrowCreated dispatches Actions.arrowCreated()', () => {
    const data = { arrowInfo: makeArrow() };
    Dispatch.arrowCreated(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.arrowCreated(1, 2, data));
  });

  it('arrowDeleted dispatches Actions.arrowDeleted()', () => {
    const data = { arrowId: 3 };
    Dispatch.arrowDeleted(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.arrowDeleted(1, 2, data));
  });

  it('counterCreated dispatches Actions.counterCreated()', () => {
    const data = { counterInfo: makeCounter() };
    Dispatch.counterCreated(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.counterCreated(1, 2, data));
  });

  it('counterSet dispatches Actions.counterSet()', () => {
    const data = { counterId: 1, value: 10 };
    Dispatch.counterSet(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.counterSet(1, 2, data));
  });

  it('counterDeleted dispatches Actions.counterDeleted()', () => {
    const data = { counterId: 1 };
    Dispatch.counterDeleted(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.counterDeleted(1, 2, data));
  });

  it('cardsDrawn dispatches Actions.cardsDrawn()', () => {
    const data = { number: 2, cards: [makeCard()] };
    Dispatch.cardsDrawn(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardsDrawn(1, 2, data));
  });

  it('cardsRevealed dispatches Actions.cardsRevealed()', () => {
    const data = { zoneName: 'hand', cards: [] } as any;
    Dispatch.cardsRevealed(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.cardsRevealed(1, 2, data));
  });

  it('zoneShuffled dispatches Actions.zoneShuffled()', () => {
    const data = { zoneName: 'deck', start: 0, end: 39 };
    Dispatch.zoneShuffled(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.zoneShuffled(1, 2, data));
  });

  it('dieRolled dispatches Actions.dieRolled()', () => {
    const data = { sides: 6, value: 4, values: [4] };
    Dispatch.dieRolled(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.dieRolled(1, 2, data));
  });

  it('activePlayerSet dispatches Actions.activePlayerSet()', () => {
    Dispatch.activePlayerSet(1, 3);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.activePlayerSet(1, 3));
  });

  it('activePhaseSet dispatches Actions.activePhaseSet()', () => {
    Dispatch.activePhaseSet(1, 2);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.activePhaseSet(1, 2));
  });

  it('turnReversed dispatches Actions.turnReversed()', () => {
    Dispatch.turnReversed(1, true);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.turnReversed(1, true));
  });

  it('zoneDumped dispatches Actions.zoneDumped()', () => {
    const data = { zoneOwnerId: 1, zoneName: 'hand', numberCards: 3, isReversed: false };
    Dispatch.zoneDumped(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.zoneDumped(1, 2, data));
  });

  it('zonePropertiesChanged dispatches Actions.zonePropertiesChanged()', () => {
    const data = { zoneName: 'deck', alwaysRevealTopCard: true, alwaysLookAtTopCard: false };
    Dispatch.zonePropertiesChanged(1, 2, data);
    expect(store.dispatch).toHaveBeenCalledWith(Actions.zonePropertiesChanged(1, 2, data));
  });

  it('gameSay dispatches Actions.gameSay()', () => {
    Dispatch.gameSay(1, 2, 'gg wp');
    expect(store.dispatch).toHaveBeenCalledWith(Actions.gameSay(1, 2, 'gg wp'));
  });
});
