vi.mock('../../services/BackendService', () => ({
  BackendService: {
    sendAdminCommand: vi.fn(),
  },
}));

vi.mock('../../persistence', () => ({
  AdminPersistence: {
    adjustMod: vi.fn(),
    reloadConfig: vi.fn(),
    shutdownServer: vi.fn(),
    updateServerMessage: vi.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';
import { adjustMod } from './adjustMod';
import { reloadConfig } from './reloadConfig';
import { shutdownServer } from './shutdownServer';
import { updateServerMessage } from './updateServerMessage';

import { Mock } from 'vitest';

const { invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendAdminCommand as Mock,
  2
);

beforeEach(() => vi.clearAllMocks());

// ----------------------------------------------------------------
// adjustMod
// ----------------------------------------------------------------
describe('adjustMod', () => {

  it('calls sendAdminCommand with Command_AdjustMod', () => {
    adjustMod('alice', true, false);
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls AdminPersistence.adjustMod', () => {
    adjustMod('alice', true, false);
    invokeOnSuccess();
    expect(AdminPersistence.adjustMod).toHaveBeenCalledWith('alice', true, false);
  });
});

// ----------------------------------------------------------------
// reloadConfig
// ----------------------------------------------------------------
describe('reloadConfig', () => {

  it('calls sendAdminCommand with Command_ReloadConfig', () => {
    reloadConfig();
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls AdminPersistence.reloadConfig', () => {
    reloadConfig();
    invokeOnSuccess();
    expect(AdminPersistence.reloadConfig).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// shutdownServer
// ----------------------------------------------------------------
describe('shutdownServer', () => {

  it('calls sendAdminCommand with Command_ShutdownServer', () => {
    shutdownServer('maintenance', 10);
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls AdminPersistence.shutdownServer', () => {
    shutdownServer('maintenance', 10);
    invokeOnSuccess();
    expect(AdminPersistence.shutdownServer).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// updateServerMessage
// ----------------------------------------------------------------
describe('updateServerMessage', () => {

  it('calls sendAdminCommand with Command_UpdateServerMessage', () => {
    updateServerMessage();
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls AdminPersistence.updateServerMessage', () => {
    updateServerMessage();
    invokeOnSuccess();
    expect(AdminPersistence.updateServerMessage).toHaveBeenCalled();
  });
});
