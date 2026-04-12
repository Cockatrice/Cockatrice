import { GamePersistence } from './GamePersistence';

describe('GamePersistence', () => {
  it('joinGame logs to console', () => {
    const spy = jest.spyOn(console, 'log').mockImplementation(() => {});
    const data = { playerId: 1 } as any;
    GamePersistence.joinGame(data);
    expect(spy).toHaveBeenCalledWith('joinGame', data);
    spy.mockRestore();
  });

  it('leaveGame logs to console', () => {
    const spy = jest.spyOn(console, 'log').mockImplementation(() => {});
    GamePersistence.leaveGame(0 as any);
    expect(spy).toHaveBeenCalledWith('leaveGame', 0);
    spy.mockRestore();
  });
});
