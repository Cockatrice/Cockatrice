jest.mock('store', () => ({
  ServerDispatch: {
    adjustMod: jest.fn(),
    reloadConfig: jest.fn(),
    shutdownServer: jest.fn(),
    updateServerMessage: jest.fn(),
  },
}));

import { AdminPersistence } from './AdminPersistence';
import { ServerDispatch } from 'store';

beforeEach(() => {
  jest.clearAllMocks();
});

describe('AdminPersistence', () => {
  it('adjustMod passes userName, shouldBeMod, shouldBeJudge', () => {
    AdminPersistence.adjustMod('alice', true, false);
    expect(ServerDispatch.adjustMod).toHaveBeenCalledWith('alice', true, false);
  });

  it('reloadConfig -> ServerDispatch.reloadConfig', () => {
    AdminPersistence.reloadConfig();
    expect(ServerDispatch.reloadConfig).toHaveBeenCalled();
  });

  it('shutdownServer -> ServerDispatch.shutdownServer', () => {
    AdminPersistence.shutdownServer();
    expect(ServerDispatch.shutdownServer).toHaveBeenCalled();
  });

  it('updateServerMessage -> ServerDispatch.updateServerMessage', () => {
    AdminPersistence.updateServerMessage();
    expect(ServerDispatch.updateServerMessage).toHaveBeenCalled();
  });
});
