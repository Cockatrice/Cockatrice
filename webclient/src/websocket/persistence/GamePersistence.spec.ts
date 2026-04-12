import { GamePersistence } from './GamePersistence';

jest.mock('store', () => ({
  GameDispatch: {
    playerJoined: jest.fn(),
    playerLeft: jest.fn(),
  },
}));

import { GameDispatch } from 'store';

beforeEach(() => jest.clearAllMocks());

describe('GamePersistence', () => {
  it('playerJoined dispatches via GameDispatch', () => {
    const data = { playerId: 1 } as any;
    GamePersistence.playerJoined(5, data);
    expect(GameDispatch.playerJoined).toHaveBeenCalledWith(5, data);
  });

  it('playerLeft dispatches via GameDispatch', () => {
    GamePersistence.playerLeft(5, 1, 3);
    expect(GameDispatch.playerLeft).toHaveBeenCalledWith(5, 1, 3);
  });
});
