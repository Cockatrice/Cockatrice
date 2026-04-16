vi.mock('../../WebClient', () => ({
  WebClient: {
    instance: {
      protobuf: { sendModeratorCommand: vi.fn() },
      response: {
        moderator: {
          banFromServer: vi.fn(),
          forceActivateUser: vi.fn(),
          getAdminNotes: vi.fn(),
          banHistory: vi.fn(),
          warnHistory: vi.fn(),
          warnListOptions: vi.fn(),
          grantReplayAccess: vi.fn(),
          updateAdminNotes: vi.fn(),
          viewLogs: vi.fn(),
          warnUser: vi.fn(),
        },
      },
    },
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { useWebClientCleanup } from '../../__mocks__/helpers';
import { WebClient } from '../../WebClient';
import {
  Command_BanFromServer_ext,
  Command_ForceActivateUser_ext,
  Command_GetAdminNotes_ext,
  Command_GetBanHistory_ext,
  Command_GetWarnHistory_ext,
  Command_GetWarnList_ext,
  Command_GrantReplayAccess_ext,
  Command_UpdateAdminNotes_ext,
  Command_ViewLogHistory_ext,
  Command_ViewLogHistorySchema,
  Command_WarnUser_ext,
  Response_BanHistory_ext,
  Response_GetAdminNotes_ext,
  Response_ViewLogHistory_ext,
  Response_WarnHistory_ext,
  Response_WarnList_ext,
} from '@app/generated';

import { banFromServer } from './banFromServer';
import { forceActivateUser } from './forceActivateUser';
import { getAdminNotes } from './getAdminNotes';
import { getBanHistory } from './getBanHistory';
import { getWarnHistory } from './getWarnHistory';
import { getWarnList } from './getWarnList';
import { grantReplayAccess } from './grantReplayAccess';
import { updateAdminNotes } from './updateAdminNotes';
import { viewLogHistory } from './viewLogHistory';
import { warnUser } from './warnUser';
import { create } from '@bufbuild/protobuf';
import { Mock } from 'vitest';

useWebClientCleanup();

const { invokeOnSuccess } = makeCallbackHelpers(
  WebClient.instance.protobuf.sendModeratorCommand as Mock,
  2
);

// ----------------------------------------------------------------
// banFromServer
// ----------------------------------------------------------------
describe('banFromServer', () => {

  it('calls sendModeratorCommand with Command_BanFromServer', () => {
    banFromServer(30, 'alice', '1.2.3.4', 'reason', 'visible', 'cid', 1);
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_BanFromServer_ext,
      expect.objectContaining({ minutes: 30, userName: 'alice' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls response.moderator.banFromServer', () => {
    banFromServer(30, 'alice');
    invokeOnSuccess();
    expect(WebClient.instance.response.moderator.banFromServer).toHaveBeenCalledWith('alice');
  });
});

// ----------------------------------------------------------------
// forceActivateUser
// ----------------------------------------------------------------
describe('forceActivateUser', () => {

  it('calls sendModeratorCommand with Command_ForceActivateUser', () => {
    forceActivateUser('alice', 'mod1');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_ForceActivateUser_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls response.moderator.forceActivateUser', () => {
    forceActivateUser('alice', 'mod1');
    invokeOnSuccess();
    expect(WebClient.instance.response.moderator.forceActivateUser).toHaveBeenCalledWith('alice', 'mod1');
  });
});

// ----------------------------------------------------------------
// getAdminNotes
// ----------------------------------------------------------------
describe('getAdminNotes', () => {

  it('calls sendModeratorCommand with Command_GetAdminNotes', () => {
    getAdminNotes('alice');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetAdminNotes_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_GetAdminNotes_ext })
    );
  });

  it('onSuccess calls response.moderator.getAdminNotes with notes', () => {
    getAdminNotes('alice');
    const resp = { notes: 'some notes' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.moderator.getAdminNotes).toHaveBeenCalledWith('alice', 'some notes');
  });
});

// ----------------------------------------------------------------
// getBanHistory
// ----------------------------------------------------------------
describe('getBanHistory', () => {

  it('calls sendModeratorCommand with Command_GetBanHistory', () => {
    getBanHistory('alice');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetBanHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_BanHistory_ext })
    );
  });

  it('onSuccess calls response.moderator.banHistory with banList', () => {
    getBanHistory('alice');
    const resp = { banList: [{ id: 1 }] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.moderator.banHistory).toHaveBeenCalledWith('alice', [{ id: 1 }]);
  });
});

// ----------------------------------------------------------------
// getWarnHistory
// ----------------------------------------------------------------
describe('getWarnHistory', () => {

  it('calls sendModeratorCommand with Command_GetWarnHistory', () => {
    getWarnHistory('alice');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetWarnHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_WarnHistory_ext })
    );
  });

  it('onSuccess calls response.moderator.warnHistory with warnList', () => {
    getWarnHistory('alice');
    const resp = { warnList: [{ id: 2 }] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.moderator.warnHistory).toHaveBeenCalledWith('alice', [{ id: 2 }]);
  });
});

// ----------------------------------------------------------------
// getWarnList
// ----------------------------------------------------------------
describe('getWarnList', () => {

  it('calls sendModeratorCommand with Command_GetWarnList', () => {
    getWarnList('mod1', 'alice', 'US');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetWarnList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_WarnList_ext })
    );
  });

  it('onSuccess calls response.moderator.warnListOptions with the response', () => {
    getWarnList('mod1', 'alice', 'US');
    const resp = { warning: ['w1', 'w2'], userName: 'alice', userClientid: 'US' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.moderator.warnListOptions).toHaveBeenCalledWith([resp]);
  });
});

// ----------------------------------------------------------------
// grantReplayAccess
// ----------------------------------------------------------------
describe('grantReplayAccess', () => {

  it('calls sendModeratorCommand with Command_GrantReplayAccess', () => {
    grantReplayAccess(10, 'mod1');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GrantReplayAccess_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls response.moderator.grantReplayAccess', () => {
    grantReplayAccess(10, 'mod1');
    invokeOnSuccess();
    expect(WebClient.instance.response.moderator.grantReplayAccess).toHaveBeenCalledWith(10, 'mod1');
  });
});

// ----------------------------------------------------------------
// updateAdminNotes
// ----------------------------------------------------------------
describe('updateAdminNotes', () => {

  it('calls sendModeratorCommand with Command_UpdateAdminNotes', () => {
    updateAdminNotes('alice', 'new notes');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_UpdateAdminNotes_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls response.moderator.updateAdminNotes', () => {
    updateAdminNotes('alice', 'new notes');
    invokeOnSuccess();
    expect(WebClient.instance.response.moderator.updateAdminNotes).toHaveBeenCalledWith('alice', 'new notes');
  });
});

// ----------------------------------------------------------------
// viewLogHistory
// ----------------------------------------------------------------
describe('viewLogHistory', () => {

  it('calls sendModeratorCommand with Command_ViewLogHistory', () => {
    const filters = create(Command_ViewLogHistorySchema, { dateRange: 7 });
    viewLogHistory(filters);
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_ViewLogHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ViewLogHistory_ext })
    );
  });

  it('onSuccess calls response.moderator.viewLogs with logMessage', () => {
    const filters = create(Command_ViewLogHistorySchema, { dateRange: 7 });
    viewLogHistory(filters);
    const resp = { logMessage: ['log1'] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(WebClient.instance.response.moderator.viewLogs).toHaveBeenCalledWith(['log1']);
  });
});

// ----------------------------------------------------------------
// warnUser
// ----------------------------------------------------------------
describe('warnUser', () => {

  it('calls sendModeratorCommand with Command_WarnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    expect(WebClient.instance.protobuf.sendModeratorCommand).toHaveBeenCalledWith(
      Command_WarnUser_ext, expect.any(Object), expect.any(Object)
    );
  });

  it('onSuccess calls response.moderator.warnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    invokeOnSuccess();
    expect(WebClient.instance.response.moderator.warnUser).toHaveBeenCalledWith('alice');
  });
});
