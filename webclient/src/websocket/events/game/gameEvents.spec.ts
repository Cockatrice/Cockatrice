jest.mock('../../persistence', () => ({
  GamePersistence: {
    playerJoined: jest.fn(),
    playerLeft: jest.fn(),
  },
}));

import { GamePersistence } from '../../persistence';
import { joinGame } from './joinGame';
import { leaveGame } from './leaveGame';

beforeEach(() => jest.clearAllMocks());

describe('joinGame event', () => {
  it('delegates to GamePersistence.playerJoined with gameId from meta', () => {
    const playerProperties = { playerId: 1 };
    const data = { playerProperties } as any;
    const meta = { gameId: 5, playerId: 1, context: null, secondsElapsed: 0, forcedByJudge: 0 };
    joinGame(data, meta);
    expect(GamePersistence.playerJoined).toHaveBeenCalledWith(5, playerProperties);
  });
});

describe('leaveGame event', () => {
  it('delegates to GamePersistence.playerLeft with gameId/playerId from meta', () => {
    const data = { reason: 3 };
    const meta = { gameId: 5, playerId: 2, context: null, secondsElapsed: 0, forcedByJudge: 0 };
    leaveGame(data, meta);
    expect(GamePersistence.playerLeft).toHaveBeenCalledWith(5, 2, 3);
  });
});
