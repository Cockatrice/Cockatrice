jest.mock('../../services/BackendService', () => ({
  BackendService: {
    sendModeratorCommand: jest.fn(),
  },
}));

jest.mock('../../persistence', () => ({
  ModeratorPersistence: {
    banFromServer: jest.fn(),
    forceActivateUser: jest.fn(),
    getAdminNotes: jest.fn(),
    banHistory: jest.fn(),
    warnHistory: jest.fn(),
    warnListOptions: jest.fn(),
    grantReplayAccess: jest.fn(),
    updateAdminNotes: jest.fn(),
    viewLogs: jest.fn(),
    warnUser: jest.fn(),
  },
}));

import { makeCallbackHelpers } from '../../__mocks__/callbackHelpers';
import { BackendService } from '../../services/BackendService';
import { ModeratorPersistence } from '../../persistence';

const { getLastSendOpts, invokeOnSuccess } = makeCallbackHelpers(
  BackendService.sendModeratorCommand as jest.Mock
);

beforeEach(() => jest.clearAllMocks());

// ----------------------------------------------------------------
// banFromServer
// ----------------------------------------------------------------
describe('banFromServer', () => {
  const { banFromServer } = jest.requireActual('./banFromServer');

  it('calls sendModeratorCommand with Command_BanFromServer', () => {
    banFromServer(30, 'alice', '1.2.3.4', 'reason', 'visible', 'cid', 1);
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_BanFromServer',
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
  const { forceActivateUser } = jest.requireActual('./forceActivateUser');

  it('calls sendModeratorCommand with Command_ForceActivateUser', () => {
    forceActivateUser('alice', 'mod1');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith('Command_ForceActivateUser', expect.any(Object), expect.any(Object));
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
  const { getAdminNotes } = jest.requireActual('./getAdminNotes');

  it('calls sendModeratorCommand with Command_GetAdminNotes', () => {
    getAdminNotes('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_GetAdminNotes',
      expect.any(Object),
      expect.objectContaining({ responseName: 'Response_GetAdminNotes' })
    );
  });

  it('onSuccess calls ModeratorPersistence.getAdminNotes with notes', () => {
    getAdminNotes('alice');
    const resp = { notes: 'some notes' };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_GetAdminNotes.ext': resp });
    expect(ModeratorPersistence.getAdminNotes).toHaveBeenCalledWith('alice', 'some notes');
  });
});

// ----------------------------------------------------------------
// getBanHistory
// ----------------------------------------------------------------
describe('getBanHistory', () => {
  const { getBanHistory } = jest.requireActual('./getBanHistory');

  it('calls sendModeratorCommand with Command_GetBanHistory', () => {
    getBanHistory('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_GetBanHistory',
      expect.any(Object),
      expect.objectContaining({ responseName: 'Response_BanHistory' })
    );
  });

  it('onSuccess calls ModeratorPersistence.banHistory with banList', () => {
    getBanHistory('alice');
    const resp = { banList: [{ id: 1 }] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_BanHistory.ext': resp });
    expect(ModeratorPersistence.banHistory).toHaveBeenCalledWith('alice', [{ id: 1 }]);
  });
});

// ----------------------------------------------------------------
// getWarnHistory
// ----------------------------------------------------------------
describe('getWarnHistory', () => {
  const { getWarnHistory } = jest.requireActual('./getWarnHistory');

  it('calls sendModeratorCommand with Command_GetWarnHistory', () => {
    getWarnHistory('alice');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_GetWarnHistory',
      expect.any(Object),
      expect.objectContaining({ responseName: 'Response_WarnHistory' })
    );
  });

  it('onSuccess calls ModeratorPersistence.warnHistory with warnList', () => {
    getWarnHistory('alice');
    const resp = { warnList: [{ id: 2 }] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_WarnHistory.ext': resp });
    expect(ModeratorPersistence.warnHistory).toHaveBeenCalledWith('alice', [{ id: 2 }]);
  });
});

// ----------------------------------------------------------------
// getWarnList
// ----------------------------------------------------------------
describe('getWarnList', () => {
  const { getWarnList } = jest.requireActual('./getWarnList');

  it('calls sendModeratorCommand with Command_GetWarnList', () => {
    getWarnList('mod1', 'alice', 'US');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_GetWarnList',
      expect.any(Object),
      expect.objectContaining({ responseName: 'Response_WarnList' })
    );
  });

  it('onSuccess calls ModeratorPersistence.warnListOptions with warning', () => {
    getWarnList('mod1', 'alice', 'US');
    const resp = { warning: ['w1', 'w2'] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_WarnList.ext': resp });
    expect(ModeratorPersistence.warnListOptions).toHaveBeenCalledWith(['w1', 'w2']);
  });
});

// ----------------------------------------------------------------
// grantReplayAccess
// ----------------------------------------------------------------
describe('grantReplayAccess', () => {
  const { grantReplayAccess } = jest.requireActual('./grantReplayAccess');

  it('calls sendModeratorCommand with Command_GrantReplayAccess', () => {
    grantReplayAccess(10, 'mod1');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith('Command_GrantReplayAccess', expect.any(Object), expect.any(Object));
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
  const { updateAdminNotes } = jest.requireActual('./updateAdminNotes');

  it('calls sendModeratorCommand with Command_UpdateAdminNotes', () => {
    updateAdminNotes('alice', 'new notes');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith('Command_UpdateAdminNotes', expect.any(Object), expect.any(Object));
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
  const { viewLogHistory } = jest.requireActual('./viewLogHistory');

  it('calls sendModeratorCommand with Command_ViewLogHistory', () => {
    viewLogHistory({ filters: 'all' } as any);
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith(
      'Command_ViewLogHistory',
      expect.any(Object),
      expect.objectContaining({ responseName: 'Response_ViewLogHistory' })
    );
  });

  it('onSuccess calls ModeratorPersistence.viewLogs with logMessage', () => {
    viewLogHistory({ filters: 'all' } as any);
    const resp = { logMessage: ['log1'] };
    invokeOnSuccess(resp, { responseCode: 0, '.Response_ViewLogHistory.ext': resp });
    expect(ModeratorPersistence.viewLogs).toHaveBeenCalledWith(['log1']);
  });
});

// ----------------------------------------------------------------
// warnUser
// ----------------------------------------------------------------
describe('warnUser', () => {
  const { warnUser } = jest.requireActual('./warnUser');

  it('calls sendModeratorCommand with Command_WarnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    expect(BackendService.sendModeratorCommand).toHaveBeenCalledWith('Command_WarnUser', expect.any(Object), expect.any(Object));
  });

  it('onSuccess calls ModeratorPersistence.warnUser', () => {
    warnUser('alice', 'bad behavior', 'cid');
    invokeOnSuccess();
    expect(ModeratorPersistence.warnUser).toHaveBeenCalledWith('alice');
  });
});
