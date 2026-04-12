jest.mock('../../persistence', () => ({
  GamePersistence: {
    joinGame: jest.fn(),
    leaveGame: jest.fn(),
  },
}));

import { GamePersistence } from '../../persistence';

beforeEach(() => jest.clearAllMocks());

describe('joinGame event', () => {
  const { joinGame } = jest.requireActual('./joinGame');

  it('delegates to GamePersistence.joinGame', () => {
    const data = { gameId: 5, player: { playerId: 1 } } as any;
    joinGame(data);
    expect(GamePersistence.joinGame).toHaveBeenCalledWith(data);
  });
});

describe('leaveGame event', () => {
  const { leaveGame } = jest.requireActual('./leaveGame');

  it('delegates to GamePersistence.leaveGame', () => {
    leaveGame(42 as any);
    expect(GamePersistence.leaveGame).toHaveBeenCalledWith(42);
  });
});
