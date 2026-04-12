jest.mock('../../persistence', () => ({
  SessionPersistence: {
    playerPropertiesChanged: jest.fn(),
  },
}));

import { SessionPersistence } from '../../persistence';

beforeEach(() => jest.clearAllMocks());

describe('playerPropertiesChanged', () => {
  const { playerPropertiesChanged } = jest.requireActual('./playerPropertiesChanged');

  it('delegates to SessionPersistence.playerPropertiesChanged', () => {
    const payload = { gameId: 1, player: { playerId: 2 } } as any;
    playerPropertiesChanged(payload);
    expect(SessionPersistence.playerPropertiesChanged).toHaveBeenCalledWith(payload);
  });
});
