vi.mock('@app/store', () => ({
  ServerDispatch: {
    adjustMod: vi.fn(),
    reloadConfig: vi.fn(),
    shutdownServer: vi.fn(),
    updateServerMessage: vi.fn(),
  },
}));

import { AdminPersistence } from './AdminPersistence';
import { ServerDispatch } from '@app/store';

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
