vi.mock('@app/store', () => ({
  ServerDispatch: {
    banFromServer: vi.fn(),
    banHistory: vi.fn(),
    viewLogs: vi.fn(),
    warnHistory: vi.fn(),
    warnListOptions: vi.fn(),
    warnUser: vi.fn(),
    grantReplayAccess: vi.fn(),
    forceActivateUser: vi.fn(),
    getAdminNotes: vi.fn(),
    updateAdminNotes: vi.fn(),
  },
}));

import { ModeratorPersistence } from './ModeratorPersistence';
import { ServerDispatch } from '@app/store';
import { create } from '@bufbuild/protobuf';
import { Data } from '@app/types';

describe('ModeratorPersistence', () => {
  it('banFromServer passes userName', () => {
    ModeratorPersistence.banFromServer('alice');
    expect(ServerDispatch.banFromServer).toHaveBeenCalledWith('alice');
  });

  it('banHistory passes userName and banHistory', () => {
    ModeratorPersistence.banHistory('alice', []);
    expect(ServerDispatch.banHistory).toHaveBeenCalledWith('alice', []);
  });

  it('viewLogs dispatches raw logs', () => {
    const logs = [create(Data.ServerInfo_ChatMessageSchema, { targetType: 'room' })];
    ModeratorPersistence.viewLogs(logs);
    expect(ServerDispatch.viewLogs).toHaveBeenCalledWith(logs);
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
