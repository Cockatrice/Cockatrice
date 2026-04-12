jest.mock('../../services/BackendService', () => ({
  BackendService: {
    sendAdminCommand: jest.fn(),
  },
}));

jest.mock('../../persistence', () => ({
  AdminPersistence: {
    adjustMod: jest.fn(),
    reloadConfig: jest.fn(),
    shutdownServer: jest.fn(),
    updateServerMessage: jest.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { AdminPersistence } from '../../persistence';

const { getLastSendOpts, invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendAdminCommand as jest.Mock
);

beforeEach(() => jest.clearAllMocks());

// ----------------------------------------------------------------
// adjustMod
// ----------------------------------------------------------------
describe('adjustMod', () => {
  const { adjustMod } = jest.requireActual('./adjustMod');

  it('calls sendAdminCommand with Command_AdjustMod', () => {
    adjustMod('alice', true, false);
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(
      'Command_AdjustMod',
      expect.objectContaining({ userName: 'alice', shouldBeMod: true, shouldBeJudge: false }),
      expect.any(Object)
    );
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
  const { reloadConfig } = jest.requireActual('./reloadConfig');

  it('calls sendAdminCommand with Command_ReloadConfig', () => {
    reloadConfig();
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith('Command_ReloadConfig', {}, expect.any(Object));
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
  const { shutdownServer } = jest.requireActual('./shutdownServer');

  it('calls sendAdminCommand with Command_ShutdownServer', () => {
    shutdownServer('maintenance', 10);
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith(
      'Command_ShutdownServer',
      { reason: 'maintenance', minutes: 10 },
      expect.any(Object)
    );
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
  const { updateServerMessage } = jest.requireActual('./updateServerMessage');

  it('calls sendAdminCommand with Command_UpdateServerMessage', () => {
    updateServerMessage();
    expect(BackendService.sendAdminCommand).toHaveBeenCalledWith('Command_UpdateServerMessage', {}, expect.any(Object));
  });

  it('onSuccess calls AdminPersistence.updateServerMessage', () => {
    updateServerMessage();
    invokeOnSuccess();
    expect(AdminPersistence.updateServerMessage).toHaveBeenCalled();
  });
});
