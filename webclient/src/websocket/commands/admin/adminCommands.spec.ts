vi.mock('../../WebClient', () => ({
  WebClient: {
    instance: {
      protobuf: { sendAdminCommand: vi.fn() },
      response: {
        admin: {
          adjustMod: vi.fn(),
          reloadConfig: vi.fn(),
          shutdownServer: vi.fn(),
          updateServerMessage: vi.fn(),
        },
      },
    },
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { useWebClientCleanup } from '../../__mocks__/helpers';
import { WebClient } from '../../WebClient';
import { adjustMod } from './adjustMod';
import { reloadConfig } from './reloadConfig';
import { shutdownServer } from './shutdownServer';
import { updateServerMessage } from './updateServerMessage';

import { Mock } from 'vitest';

useWebClientCleanup();

const { invokeOnSuccess } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendAdminCommand as Mock,
  2
);

// ----------------------------------------------------------------
// adjustMod
// ----------------------------------------------------------------
describe('adjustMod', () => {

  it('calls sendAdminCommand with Command_AdjustMod', () => {
    adjustMod('alice', true, false);
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls response.admin.adjustMod', () => {
    adjustMod('alice', true, false);
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.adjustMod).toHaveBeenCalledWith('alice', true, false);
  });
});

// ----------------------------------------------------------------
// reloadConfig
// ----------------------------------------------------------------
describe('reloadConfig', () => {

  it('calls sendAdminCommand with Command_ReloadConfig', () => {
    reloadConfig();
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls response.admin.reloadConfig', () => {
    reloadConfig();
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.reloadConfig).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// shutdownServer
// ----------------------------------------------------------------
describe('shutdownServer', () => {

  it('calls sendAdminCommand with Command_ShutdownServer', () => {
    shutdownServer('maintenance', 10);
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls response.admin.shutdownServer', () => {
    shutdownServer('maintenance', 10);
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.shutdownServer).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// updateServerMessage
// ----------------------------------------------------------------
describe('updateServerMessage', () => {

  it('calls sendAdminCommand with Command_UpdateServerMessage', () => {
    updateServerMessage();
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(expect.any(Object), expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls response.admin.updateServerMessage', () => {
    updateServerMessage();
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.updateServerMessage).toHaveBeenCalled();
  });
});
