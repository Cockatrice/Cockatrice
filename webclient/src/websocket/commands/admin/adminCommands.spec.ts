vi.mock('../../WebClient');

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { WebClient } from '../../WebClient';
import { adjustMod } from './adjustMod';
import { reloadConfig } from './reloadConfig';
import { shutdownServer } from './shutdownServer';
import { updateServerMessage } from './updateServerMessage';
import {
  Command_AdjustMod_ext,
  Command_ReloadConfig_ext,
  Command_ShutdownServer_ext,
  Command_UpdateServerMessage_ext,
} from '@app/generated';

import { Mock } from 'vitest';

const { invokeOnSuccess } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendAdminCommand as Mock,
  2
);

describe('adjustMod', () => {

  it('calls sendAdminCommand with Command_AdjustMod extension and fields', () => {
    adjustMod('alice', true, false);
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(
      Command_AdjustMod_ext,
      expect.objectContaining({ userName: 'alice', shouldBeMod: true, shouldBeJudge: false }),
      expect.any(Object)
    );
  });

  it('onSuccess calls response.admin.adjustMod', () => {
    adjustMod('alice', true, false);
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.adjustMod).toHaveBeenCalledWith('alice', true, false);
  });
});

describe('reloadConfig', () => {

  it('calls sendAdminCommand with Command_ReloadConfig extension', () => {
    reloadConfig();
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(
      Command_ReloadConfig_ext,
      expect.any(Object),
      expect.any(Object)
    );
  });

  it('onSuccess calls response.admin.reloadConfig', () => {
    reloadConfig();
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.reloadConfig).toHaveBeenCalled();
  });
});

describe('shutdownServer', () => {

  it('calls sendAdminCommand with Command_ShutdownServer extension and fields', () => {
    shutdownServer('maintenance', 10);
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(
      Command_ShutdownServer_ext,
      expect.objectContaining({ reason: 'maintenance', minutes: 10 }),
      expect.any(Object)
    );
  });

  it('onSuccess calls response.admin.shutdownServer', () => {
    shutdownServer('maintenance', 10);
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.shutdownServer).toHaveBeenCalled();
  });
});

describe('updateServerMessage', () => {

  it('calls sendAdminCommand with Command_UpdateServerMessage extension', () => {
    updateServerMessage();
    expect(WebClient.instance.protobuf.sendAdminCommand).toHaveBeenCalledWith(
      Command_UpdateServerMessage_ext,
      expect.any(Object),
      expect.any(Object)
    );
  });

  it('onSuccess calls response.admin.updateServerMessage', () => {
    updateServerMessage();
    invokeOnSuccess();
    expect(WebClient.instance.response.admin.updateServerMessage).toHaveBeenCalled();
  });
});
