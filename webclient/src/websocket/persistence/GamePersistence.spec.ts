import { GamePersistence } from './GamePersistence';

jest.mock('store', () => ({
  GameDispatch: {
    gameStateChanged: jest.fn(),
    playerJoined: jest.fn(),
    playerLeft: jest.fn(),
    playerPropertiesChanged: jest.fn(),
    gameClosed: jest.fn(),
    gameHostChanged: jest.fn(),
    kicked: jest.fn(),
    gameSay: jest.fn(),
    cardMoved: jest.fn(),
    cardFlipped: jest.fn(),
    cardDestroyed: jest.fn(),
    cardAttached: jest.fn(),
    tokenCreated: jest.fn(),
    cardAttrChanged: jest.fn(),
    cardCounterChanged: jest.fn(),
    arrowCreated: jest.fn(),
    arrowDeleted: jest.fn(),
    counterCreated: jest.fn(),
    counterSet: jest.fn(),
    counterDeleted: jest.fn(),
    cardsDrawn: jest.fn(),
    cardsRevealed: jest.fn(),
    zoneShuffled: jest.fn(),
    dieRolled: jest.fn(),
    activePlayerSet: jest.fn(),
    activePhaseSet: jest.fn(),
    turnReversed: jest.fn(),
    zoneDumped: jest.fn(),
    zonePropertiesChanged: jest.fn(),
  },
}));

import { GameDispatch } from 'store';

beforeEach(() => jest.clearAllMocks());

describe('GamePersistence', () => {
  it('gameStateChanged dispatches via GameDispatch', () => {
    const data = { playerList: [] } as any;
    GamePersistence.gameStateChanged(5, data);
    expect(GameDispatch.gameStateChanged).toHaveBeenCalledWith(5, data);
  });

  it('playerJoined dispatches via GameDispatch', () => {
    const data = { playerId: 1 } as any;
    GamePersistence.playerJoined(5, data);
    expect(GameDispatch.playerJoined).toHaveBeenCalledWith(5, data);
  });

  it('playerLeft dispatches via GameDispatch', () => {
    GamePersistence.playerLeft(5, 1, 3);
    expect(GameDispatch.playerLeft).toHaveBeenCalledWith(5, 1, 3);
  });

  it('playerPropertiesChanged dispatches via GameDispatch', () => {
    const props = { playerId: 2 } as any;
    GamePersistence.playerPropertiesChanged(5, 2, props);
    expect(GameDispatch.playerPropertiesChanged).toHaveBeenCalledWith(5, 2, props);
  });

  it('gameClosed dispatches via GameDispatch', () => {
    GamePersistence.gameClosed(5);
    expect(GameDispatch.gameClosed).toHaveBeenCalledWith(5);
  });

  it('gameHostChanged dispatches via GameDispatch', () => {
    GamePersistence.gameHostChanged(5, 7);
    expect(GameDispatch.gameHostChanged).toHaveBeenCalledWith(5, 7);
  });

  it('kicked dispatches via GameDispatch', () => {
    GamePersistence.kicked(5);
    expect(GameDispatch.kicked).toHaveBeenCalledWith(5);
  });

  it('gameSay dispatches via GameDispatch', () => {
    GamePersistence.gameSay(5, 1, 'hello');
    expect(GameDispatch.gameSay).toHaveBeenCalledWith(5, 1, 'hello');
  });

  it('cardMoved dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardMoved(5, 1, data);
    expect(GameDispatch.cardMoved).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardFlipped dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardFlipped(5, 1, data);
    expect(GameDispatch.cardFlipped).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardDestroyed dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardDestroyed(5, 1, data);
    expect(GameDispatch.cardDestroyed).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardAttached dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardAttached(5, 1, data);
    expect(GameDispatch.cardAttached).toHaveBeenCalledWith(5, 1, data);
  });

  it('tokenCreated dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.tokenCreated(5, 1, data);
    expect(GameDispatch.tokenCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardAttrChanged dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardAttrChanged(5, 1, data);
    expect(GameDispatch.cardAttrChanged).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardCounterChanged dispatches via GameDispatch', () => {
    const data = { cardId: 3 } as any;
    GamePersistence.cardCounterChanged(5, 1, data);
    expect(GameDispatch.cardCounterChanged).toHaveBeenCalledWith(5, 1, data);
  });

  it('arrowCreated dispatches via GameDispatch', () => {
    const data = { arrowInfo: {} } as any;
    GamePersistence.arrowCreated(5, 1, data);
    expect(GameDispatch.arrowCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('arrowDeleted dispatches via GameDispatch', () => {
    const data = { arrowId: 9 };
    GamePersistence.arrowDeleted(5, 1, data);
    expect(GameDispatch.arrowDeleted).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterCreated dispatches via GameDispatch', () => {
    const data = { counterInfo: {} } as any;
    GamePersistence.counterCreated(5, 1, data);
    expect(GameDispatch.counterCreated).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterSet dispatches via GameDispatch', () => {
    const data = { counterId: 1, value: 20 };
    GamePersistence.counterSet(5, 1, data);
    expect(GameDispatch.counterSet).toHaveBeenCalledWith(5, 1, data);
  });

  it('counterDeleted dispatches via GameDispatch', () => {
    const data = { counterId: 1 };
    GamePersistence.counterDeleted(5, 1, data);
    expect(GameDispatch.counterDeleted).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardsDrawn dispatches via GameDispatch', () => {
    const data = { number: 2, cards: [] } as any;
    GamePersistence.cardsDrawn(5, 1, data);
    expect(GameDispatch.cardsDrawn).toHaveBeenCalledWith(5, 1, data);
  });

  it('cardsRevealed dispatches via GameDispatch', () => {
    const data = { zoneName: 'hand', cards: [] } as any;
    GamePersistence.cardsRevealed(5, 1, data);
    expect(GameDispatch.cardsRevealed).toHaveBeenCalledWith(5, 1, data);
  });

  it('zoneShuffled dispatches via GameDispatch', () => {
    const data = { zoneName: 'deck' } as any;
    GamePersistence.zoneShuffled(5, 1, data);
    expect(GameDispatch.zoneShuffled).toHaveBeenCalledWith(5, 1, data);
  });

  it('dieRolled dispatches via GameDispatch', () => {
    const data = { die: 6, result: 4 } as any;
    GamePersistence.dieRolled(5, 1, data);
    expect(GameDispatch.dieRolled).toHaveBeenCalledWith(5, 1, data);
  });

  it('activePlayerSet dispatches via GameDispatch', () => {
    GamePersistence.activePlayerSet(5, 2);
    expect(GameDispatch.activePlayerSet).toHaveBeenCalledWith(5, 2);
  });

  it('activePhaseSet dispatches via GameDispatch', () => {
    GamePersistence.activePhaseSet(5, 3);
    expect(GameDispatch.activePhaseSet).toHaveBeenCalledWith(5, 3);
  });

  it('turnReversed dispatches via GameDispatch', () => {
    GamePersistence.turnReversed(5, true);
    expect(GameDispatch.turnReversed).toHaveBeenCalledWith(5, true);
  });

  it('zoneDumped dispatches via GameDispatch', () => {
    const data = { zoneName: 'hand' } as any;
    GamePersistence.zoneDumped(5, 1, data);
    expect(GameDispatch.zoneDumped).toHaveBeenCalledWith(5, 1, data);
  });

  it('zonePropertiesChanged dispatches via GameDispatch', () => {
    const data = { zoneName: 'hand', alwaysRevealTopCard: true } as any;
    GamePersistence.zonePropertiesChanged(5, 1, data);
    expect(GameDispatch.zonePropertiesChanged).toHaveBeenCalledWith(5, 1, data);
  });
});
