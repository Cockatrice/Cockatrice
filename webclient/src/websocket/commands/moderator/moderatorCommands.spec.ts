vi.mock('../../services/BackendService', () => ({
  BackendService: {
    sendModeratorCommand: vi.fn(),
  },
}));

vi.mock('../../persistence', () => ({
  ModeratorPersistence: {
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
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';
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
  Command_WarnUser_ext,
} from 'generated/proto/moderator_commands_pb';
import { Response_GetAdminNotes_ext } from 'generated/proto/response_get_admin_notes_pb';
import { Response_BanHistory_ext } from 'generated/proto/response_ban_history_pb';
import { Response_WarnHistory_ext } from 'generated/proto/response_warn_history_pb';
import { Response_WarnList_ext } from 'generated/proto/response_warn_list_pb';
import { Response_ViewLogHistory_ext } from 'generated/proto/response_viewlog_history_pb';
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

const { getLastSendOpts, invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendModeratorCommand as vi.Mock,
  2
);

beforeEach(() => vi.clearAllMocks());

// ----------------------------------------------------------------
// banFromServer
// ----------------------------------------------------------------
describe('banFromServer', () => {

  it('calls sendModeratorCommand with Command_BanFromServer', () => {
    banFromServer(30, 'alice', '1.2.3.4', 'reason', 'visible', 'cid', 1);
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_BanFromServer_ext,
      expect.objectContaining({ minutes: 30, userName: 'alice' }),
      expect.any(Object)
    );
  });

  it('onSuccess calls ModeratorPersistence.banFromServer', () => {
    banFromServer(30, 'alice');
    invokeOnSuccess();
    expect(ModeratorPersistence.banFromServer).toHaveBeenCalledWith('alice');
  });
});

// ----------------------------------------------------------------
// forceActivateUser
// ----------------------------------------------------------------
describe('forceActivateUser', () => {

  it('calls sendModeratorCommand with Command_ForceActivateUser', () => {
    forceActivateUser('alice', 'mod1');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(Command_ForceActivateUser_ext, expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls ModeratorPersistence.forceActivateUser', () => {
    forceActivateUser('alice', 'mod1');
    invokeOnSuccess();
    expect(ModeratorPersistence.forceActivateUser).toHaveBeenCalledWith('alice', 'mod1');
  });
});

// ----------------------------------------------------------------
// getAdminNotes
// ----------------------------------------------------------------
describe('getAdminNotes', () => {

  it('calls sendModeratorCommand with Command_GetAdminNotes', () => {
    getAdminNotes('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetAdminNotes_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_GetAdminNotes_ext })
    );
  });

  it('onSuccess calls ModeratorPersistence.getAdminNotes with notes', () => {
    getAdminNotes('alice');
    const resp = { notes: 'some notes' };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(ModeratorPersistence.getAdminNotes).toHaveBeenCalledWith('alice', 'some notes');
  });
});

// ----------------------------------------------------------------
// getBanHistory
// ----------------------------------------------------------------
describe('getBanHistory', () => {

  it('calls sendModeratorCommand with Command_GetBanHistory', () => {
    getBanHistory('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetBanHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_BanHistory_ext })
    );
  });

  it('onSuccess calls ModeratorPersistence.banHistory with banList', () => {
    getBanHistory('alice');
    const resp = { banList: [{ id: 1 }] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(ModeratorPersistence.banHistory).toHaveBeenCalledWith('alice', [{ id: 1 }]);
  });
});

// ----------------------------------------------------------------
// getWarnHistory
// ----------------------------------------------------------------
describe('getWarnHistory', () => {

  it('calls sendModeratorCommand with Command_GetWarnHistory', () => {
    getWarnHistory('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetWarnHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_WarnHistory_ext })
    );
  });

  it('onSuccess calls ModeratorPersistence.warnHistory with warnList', () => {
    getWarnHistory('alice');
    const resp = { warnList: [{ id: 2 }] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(ModeratorPersistence.warnHistory).toHaveBeenCalledWith('alice', [{ id: 2 }]);
  });
});

// ----------------------------------------------------------------
// getWarnList
// ----------------------------------------------------------------
describe('getWarnList', () => {

  it('calls sendModeratorCommand with Command_GetWarnList', () => {
    getWarnList('mod1', 'alice', 'US');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_GetWarnList_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_WarnList_ext })
    );
  });

  it('onSuccess calls ModeratorPersistence.warnListOptions with warning', () => {
    getWarnList('mod1', 'alice', 'US');
    const resp = { warning: ['w1', 'w2'] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(ModeratorPersistence.warnListOptions).toHaveBeenCalledWith(['w1', 'w2']);
  });
});

// ----------------------------------------------------------------
// grantReplayAccess
// ----------------------------------------------------------------
describe('grantReplayAccess', () => {

  it('calls sendModeratorCommand with Command_GrantReplayAccess', () => {
    grantReplayAccess(10, 'mod1');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(Command_GrantReplayAccess_ext, expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls ModeratorPersistence.grantReplayAccess', () => {
    grantReplayAccess(10, 'mod1');
    invokeOnSuccess();
    expect(ModeratorPersistence.grantReplayAccess).toHaveBeenCalledWith(10, 'mod1');
  });
});

// ----------------------------------------------------------------
// updateAdminNotes
// ----------------------------------------------------------------
describe('updateAdminNotes', () => {

  it('calls sendModeratorCommand with Command_UpdateAdminNotes', () => {
    updateAdminNotes('alice', 'new notes');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(Command_UpdateAdminNotes_ext, expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls ModeratorPersistence.updateAdminNotes', () => {
    updateAdminNotes('alice', 'new notes');
    invokeOnSuccess();
    expect(ModeratorPersistence.updateAdminNotes).toHaveBeenCalledWith('alice', 'new notes');
  });
});

// ----------------------------------------------------------------
// viewLogHistory
// ----------------------------------------------------------------
describe('viewLogHistory', () => {

  it('calls sendModeratorCommand with Command_ViewLogHistory', () => {
    viewLogHistory({ dateRange: 7 } as any);
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      Command_ViewLogHistory_ext,
      expect.any(Object),
      expect.objectContaining({ responseExt: Response_ViewLogHistory_ext })
    );
  });

  it('onSuccess calls ModeratorPersistence.viewLogs with logMessage', () => {
    viewLogHistory({ dateRange: 7 } as any);
    const resp = { logMessage: ['log1'] };
    invokeOnSuccess(resp, { responseCode: 0 });
    expect(ModeratorPersistence.viewLogs).toHaveBeenCalledWith(['log1']);
  });
});

// ----------------------------------------------------------------
// warnUser
// ----------------------------------------------------------------
describe('warnUser', () => {

  it('calls sendModeratorCommand with Command_WarnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(Command_WarnUser_ext, expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls ModeratorPersistence.warnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    invokeOnSuccess();
    expect(ModeratorPersistence.warnUser).toHaveBeenCalledWith('alice');
  });
});
