// Tests for simple session events that delegate 1:1 to SessionPersistence
// or RoomPersistence with minimal logic.

vi.mock('../../WebClient', () => ({
  WebClient: {
    instance: {
      config: { onServerIdentified: vi.fn() },
      response: {
        session: {
          gameJoined: vi.fn(),
          notifyUser: vi.fn(),
          replayAdded: vi.fn(),
          serverMessage: vi.fn(),
          serverShutdown: vi.fn(),
          updateUsers: vi.fn(),
          updateInfo: vi.fn(),
          userJoined: vi.fn(),
          userLeft: vi.fn(),
          userMessage: vi.fn(),
          addToBuddyList: vi.fn(),
          addToIgnoreList: vi.fn(),
          removeFromBuddyList: vi.fn(),
          removeFromIgnoreList: vi.fn(),
          playerPropertiesChanged: vi.fn(),
        },
        room: {
          updateRooms: vi.fn(),
        },
      },
    },
  },
}));

vi.mock('../../config', () => ({
  CLIENT_OPTIONS: { autojoinrooms: false },
}));

vi.mock('../../commands/session', () => ({
  joinRoom: vi.fn(),
  updateStatus: vi.fn(),
  disconnect: vi.fn(),
}));

vi.mock('../../utils', () => ({
  sanitizeHtml: vi.fn((msg: string) => msg),
}));

import { useWebClientCleanup } from '../../__mocks__/helpers';
import {
  Event_AddToListSchema,
  Event_ConnectionClosedSchema,
  Event_ConnectionClosed_CloseReason,
  Event_GameJoinedSchema,
  Event_ListRoomsSchema,
  Event_NotifyUserSchema,
  Event_RemoveFromListSchema,
  Event_ReplayAddedSchema,
  Event_ServerCompleteListSchema,
  Event_ServerIdentificationSchema,
  Event_ServerMessageSchema,
  Event_ServerShutdownSchema,
  Event_UserJoinedSchema,
  Event_UserLeftSchema,
  Event_UserMessageSchema,
  ServerInfo_ReplayMatchSchema,
  ServerInfo_RoomSchema,
  ServerInfo_UserSchema,
} from '@app/generated';
import { create } from '@bufbuild/protobuf';

import { WebClient } from '../../WebClient';
import * as Config from '../../config';
import * as SessionCmds from '../../commands/session';
import { gameJoined } from './gameJoined';
import { notifyUser } from './notifyUser';
import { replayAdded } from './replayAdded';
import { serverCompleteList } from './serverCompleteList';
import { serverMessage } from './serverMessage';
import { serverShutdown } from './serverShutdown';
import { userJoined } from './userJoined';
import { userLeft } from './userLeft';
import { userMessage } from './userMessage';
import { addToList } from './addToList';
import { removeFromList } from './removeFromList';
import { listRooms } from './listRooms';
import { connectionClosed } from './connectionClosed';
import { serverIdentification } from './serverIdentification';

useWebClientCleanup();

const ConfigMock = Config as { -readonly [K in keyof typeof Config]: (typeof Config)[K] };

// ----------------------------------------------------------------
// gameJoined
// ----------------------------------------------------------------
describe('gameJoined', () => {

  it('calls WebClient.instance.response.session.gameJoined', () => {
    const data = create(Event_GameJoinedSchema, { playerId: 1 });
    gameJoined(data);
    expect(WebClient.instance.response.session.gameJoined).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// notifyUser
// ----------------------------------------------------------------
describe('notifyUser', () => {

  it('calls WebClient.instance.response.session.notifyUser', () => {
    const data = create(Event_NotifyUserSchema, { warningReason: 'yo' });
    notifyUser(data);
    expect(WebClient.instance.response.session.notifyUser).toHaveBeenCalledWith(data);
  });
});

// ----------------------------------------------------------------
// replayAdded
// ----------------------------------------------------------------
describe('replayAdded', () => {

  it('calls WebClient.instance.response.session.replayAdded with matchInfo', () => {
    const data = create(Event_ReplayAddedSchema, {
      matchInfo: create(ServerInfo_ReplayMatchSchema, { gameId: 42 }),
    });
    replayAdded(data);
    expect(WebClient.instance.response.session.replayAdded).toHaveBeenCalledWith(data.matchInfo);
  });
});

// ----------------------------------------------------------------
// serverCompleteList
// ----------------------------------------------------------------
describe('serverCompleteList', () => {

  it('calls WebClient.instance.response.session.updateUsers and WebClient.instance.response.room.updateRooms', () => {
    const data = create(Event_ServerCompleteListSchema, { userList: [], roomList: [] });
    serverCompleteList(data);
    expect(WebClient.instance.response.session.updateUsers).toHaveBeenCalledWith(data.userList);
    expect(WebClient.instance.response.room.updateRooms).toHaveBeenCalledWith(data.roomList);
  });
});

// ----------------------------------------------------------------
// serverMessage
// ----------------------------------------------------------------
describe('serverMessage', () => {

  it('calls WebClient.instance.response.session.serverMessage with message', () => {
    serverMessage(create(Event_ServerMessageSchema, { message: 'hello server' }));
    expect(WebClient.instance.response.session.serverMessage).toHaveBeenCalledWith('hello server');
  });
});

// ----------------------------------------------------------------
// serverShutdown
// ----------------------------------------------------------------
describe('serverShutdown', () => {

  it('calls WebClient.instance.response.session.serverShutdown', () => {
    const payload = create(Event_ServerShutdownSchema, { reason: 'maintenance' });
    serverShutdown(payload);
    expect(WebClient.instance.response.session.serverShutdown).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// userJoined
// ----------------------------------------------------------------
describe('userJoined', () => {

  it('calls WebClient.instance.response.session.userJoined with userInfo', () => {
    const data = create(Event_UserJoinedSchema, {
      userInfo: create(ServerInfo_UserSchema, { name: 'alice' }),
    });
    userJoined(data);
    expect(WebClient.instance.response.session.userJoined).toHaveBeenCalledWith(data.userInfo);
  });
});

// ----------------------------------------------------------------
// userLeft
// ----------------------------------------------------------------
describe('userLeft', () => {

  it('calls WebClient.instance.response.session.userLeft with name', () => {
    userLeft(create(Event_UserLeftSchema, { name: 'bob' }));
    expect(WebClient.instance.response.session.userLeft).toHaveBeenCalledWith('bob');
  });
});

// ----------------------------------------------------------------
// userMessage
// ----------------------------------------------------------------
describe('userMessage', () => {

  it('calls WebClient.instance.response.session.userMessage', () => {
    const payload = create(Event_UserMessageSchema, { senderName: 'alice', message: 'hi' });
    userMessage(payload);
    expect(WebClient.instance.response.session.userMessage).toHaveBeenCalledWith(payload);
  });
});

// ----------------------------------------------------------------
// addToList
// ----------------------------------------------------------------
describe('addToList', () => {
  let logSpy: ReturnType<typeof vi.spyOn>;
  beforeEach(() => {
    logSpy = vi.spyOn(console, 'log').mockImplementation(() => {});
  });
  afterEach(() => {
    logSpy.mockRestore();
  });

  it('buddy list → addToBuddyList', () => {
    const data = create(Event_AddToListSchema, {
      listName: 'buddy',
      userInfo: create(ServerInfo_UserSchema, { name: 'alice' }),
    });
    addToList(data);
    expect(WebClient.instance.response.session.addToBuddyList).toHaveBeenCalledWith(data.userInfo);
  });

  it('ignore list → addToIgnoreList', () => {
    const data = create(Event_AddToListSchema, {
      listName: 'ignore',
      userInfo: create(ServerInfo_UserSchema, { name: 'bob' }),
    });
    addToList(data);
    expect(WebClient.instance.response.session.addToIgnoreList).toHaveBeenCalledWith(data.userInfo);
  });

  it('unknown list → console.log', () => {
    addToList(create(Event_AddToListSchema, { listName: 'unknown' }));
    expect(logSpy).toHaveBeenCalled();
  });
});

// ----------------------------------------------------------------
// removeFromList
// ----------------------------------------------------------------
describe('removeFromList', () => {

  it('buddy list → removeFromBuddyList', () => {
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'buddy', userName: 'alice' }));
    expect(WebClient.instance.response.session.removeFromBuddyList).toHaveBeenCalledWith('alice');
  });

  it('ignore list → removeFromIgnoreList', () => {
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'ignore', userName: 'bob' }));
    expect(WebClient.instance.response.session.removeFromIgnoreList).toHaveBeenCalledWith('bob');
  });

  it('unknown list → console.log', () => {
    const logSpy = vi.spyOn(console, 'log').mockImplementation(() => {});
    removeFromList(create(Event_RemoveFromListSchema, { listName: 'other', userName: 'x' }));
    expect(logSpy).toHaveBeenCalled();
    logSpy.mockRestore();
  });
});

// ----------------------------------------------------------------
// listRooms
// ----------------------------------------------------------------
describe('listRooms', () => {

  it('calls WebClient.instance.response.room.updateRooms', () => {
    listRooms(create(Event_ListRoomsSchema, { roomList: [] }));
    expect(WebClient.instance.response.room.updateRooms).toHaveBeenCalledWith([]);
  });

  it('does not call joinRoom when autojoinrooms is false', () => {
    ConfigMock.CLIENT_OPTIONS = { autojoinrooms: false };
    listRooms(create(Event_ListRoomsSchema, {
      roomList: [create(ServerInfo_RoomSchema, { autoJoin: true, roomId: 1 })]
    }));
    expect(SessionCmds.joinRoom).not.toHaveBeenCalled();
  });

  it('calls joinRoom for autoJoin rooms when autojoinrooms is true', () => {
    ConfigMock.CLIENT_OPTIONS = { autojoinrooms: true };
    listRooms(create(Event_ListRoomsSchema, {
      roomList: [
        create(ServerInfo_RoomSchema, { autoJoin: true, roomId: 2 }),
        create(ServerInfo_RoomSchema, { autoJoin: false, roomId: 3 })
      ]
    }));
    expect(SessionCmds.joinRoom).toHaveBeenCalledTimes(1);
    expect(SessionCmds.joinRoom).toHaveBeenCalledWith(2);
  });
});

// ----------------------------------------------------------------
// connectionClosed
// ----------------------------------------------------------------
describe('connectionClosed', () => {

  it('uses reasonStr when provided', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: 0, reasonStr: 'custom' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom');
  });

  it('USER_LIMIT_REACHED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.USER_LIMIT_REACHED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('maximum user capacity')
    );
  });

  it('TOO_MANY_CONNECTIONS → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.TOO_MANY_CONNECTIONS }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('too many concurrent'));
  });

  it('BANNED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('DEMOTED → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.DEMOTED }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('demoted'));
  });

  it('SERVER_SHUTDOWN → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.SERVER_SHUTDOWN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('shutdown'));
  });

  it('USERNAMEINVALID → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.USERNAMEINVALID }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('username'));
  });

  it('LOGGEDINELSEWERE → specific message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.LOGGEDINELSEWERE }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), expect.stringContaining('logged out'));
  });

  it('OTHER → "Unknown reason"', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.OTHER }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'Unknown reason');
  });

  it('BANNED with valid positive endTime → shows formatted date', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, {
      reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 1700000000,
    }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(
      expect.anything(),
      expect.stringContaining('You are banned until')
    );
  });

  it('BANNED with endTime = 0 → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 0 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = -1 → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: -1 }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = NaN → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: NaN }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with endTime = Infinity → shows generic banned message', () => {
    connectionClosed(create(Event_ConnectionClosedSchema, {
      reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: Infinity,
    }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'You are banned');
  });

  it('BANNED with reasonStr → uses reasonStr regardless of endTime', () => {
    connectionClosed(create(Event_ConnectionClosedSchema,
      { reason: Event_ConnectionClosed_CloseReason.BANNED, endTime: 0, reasonStr: 'custom ban reason' }));
    expect(SessionCmds.updateStatus).toHaveBeenCalledWith(expect.anything(), 'custom ban reason');
  });
});

// ----------------------------------------------------------------
// serverIdentification
// ----------------------------------------------------------------
describe('serverIdentification', () => {

  it('calls config.onServerIdentified with the event info', () => {
    const info = create(Event_ServerIdentificationSchema,
      { serverName: 's', serverVersion: '1', protocolVersion: 14, serverOptions: 0 });
    serverIdentification(info);
    expect(WebClient.instance.config.onServerIdentified).toHaveBeenCalledWith(info);
  });
});
