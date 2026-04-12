jest.mock('store', () => ({
  ServerDispatch: {
    banFromServer: jest.fn(),
    banHistory: jest.fn(),
    viewLogs: jest.fn(),
    warnHistory: jest.fn(),
    warnListOptions: jest.fn(),
    warnUser: jest.fn(),
    grantReplayAccess: jest.fn(),
    forceActivateUser: jest.fn(),
    getAdminNotes: jest.fn(),
    updateAdminNotes: jest.fn(),
  },
}));

jest.mock('../utils/NormalizeService', () => ({
  __esModule: true,
  default: {
    normalizeLogs: jest.fn((logs: any) => ({ normalized: logs })),
  },
}));

import { ModeratorPersistence } from './ModeratorPersistence';
import { ServerDispatch } from 'store';
import NormalizeService from '../utils/NormalizeService';

beforeEach(() => {
  jest.clearAllMocks();
  (NormalizeService.normalizeLogs as jest.Mock).mockImplementation((logs: any) => ({ normalized: logs }));
});

describe('ModeratorPersistence', () => {
  it('banFromServer passes userName', () => {
    ModeratorPersistence.banFromServer('alice');
    expect(ServerDispatch.banFromServer).toHaveBeenCalledWith('alice');
  });

  it('banHistory passes userName and banHistory', () => {
    ModeratorPersistence.banHistory('alice', []);
    expect(ServerDispatch.banHistory).toHaveBeenCalledWith('alice', []);
  });

  it('viewLogs normalizes logs and dispatches', () => {
    const logs = [{ targetType: 'room' }] as any;
    ModeratorPersistence.viewLogs(logs);
    expect(NormalizeService.normalizeLogs).toHaveBeenCalledWith(logs);
    expect(ServerDispatch.viewLogs).toHaveBeenCalledWith({ normalized: logs });
  });

  it('warnHistory passes userName and warnHistory', () => {
    ModeratorPersistence.warnHistory('bob', []);
    expect(ServerDispatch.warnHistory).toHaveBeenCalledWith('bob', []);
  });

  it('warnListOptions passes warnList', () => {
    ModeratorPersistence.warnListOptions([]);
    expect(ServerDispatch.warnListOptions).toHaveBeenCalledWith([]);
  });

  it('warnUser passes userName', () => {
    ModeratorPersistence.warnUser('carol');
    expect(ServerDispatch.warnUser).toHaveBeenCalledWith('carol');
  });

  it('grantReplayAccess passes replayId and moderatorName', () => {
    ModeratorPersistence.grantReplayAccess(10, 'mod1');
    expect(ServerDispatch.grantReplayAccess).toHaveBeenCalledWith(10, 'mod1');
  });

  it('forceActivateUser passes usernameToActivate and moderatorName', () => {
    ModeratorPersistence.forceActivateUser('user1', 'mod1');
    expect(ServerDispatch.forceActivateUser).toHaveBeenCalledWith('user1', 'mod1');
  });

  it('getAdminNotes passes userName and notes', () => {
    ModeratorPersistence.getAdminNotes('alice', 'some notes');
    expect(ServerDispatch.getAdminNotes).toHaveBeenCalledWith('alice', 'some notes');
  });

  it('updateAdminNotes passes userName and notes', () => {
    ModeratorPersistence.updateAdminNotes('alice', 'new notes');
    expect(ServerDispatch.updateAdminNotes).toHaveBeenCalledWith('alice', 'new notes');
  });
});
