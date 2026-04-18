import { App, Data } from '@app/types';
import { WebsocketTypes } from '@app/websocket/types';
import type { RootState } from '../store/store';

/**
 * Create a minimal ServerInfo_User object for testing.
 */
function makeUser(overrides: Partial<Data.ServerInfo_User> = {}): Data.ServerInfo_User {
  return {
    name: 'testUser',
    realName: '',
    country: 'us',
    userLevel: 0,
    avatarBmp: new Uint8Array(),
    accountageSecs: BigInt(0),
    $typeName: 'ServerInfo_User' as any,
    $unknown: undefined,
    gender: 0,
    ...overrides,
  } as Data.ServerInfo_User;
}

/**
 * A disconnected (default) store state. This is the state before any
 * connection to a server has been made.
 */
export const disconnectedState: Partial<RootState> = {
  server: {
    initialized: false,
    buddyList: {},
    ignoreList: {},
    status: {
      connectionAttemptMade: false,
      state: WebsocketTypes.StatusEnum.DISCONNECTED,
      description: null,
    },
    info: { message: null, name: null, version: null },
    logs: { room: [], game: [], chat: [] },
    user: null,
    users: {},
    sortUsersBy: { field: App.UserSortField.NAME, order: App.SortDirection.ASC },
    messages: {},
    userInfo: {},
    notifications: [],
    serverShutdown: null,
    banUser: '',
    banHistory: {},
    warnHistory: {},
    warnListOptions: [],
    warnUser: '',
    adminNotes: {},
    replays: {},
    backendDecks: null,
    downloadedDeck: null,
    downloadedReplay: null,
    gamesOfUser: {},
    registrationError: null,
  },
  rooms: {
    rooms: {},
    joinedRoomIds: {},
    joinedGameIds: {},
    messages: {},
    sortGamesBy: { field: App.GameSortField.START_TIME, order: App.SortDirection.DESC },
    sortUsersBy: { field: App.UserSortField.NAME, order: App.SortDirection.ASC },
  },
  games: { games: {} },
  action: { type: null, payload: null, meta: null, error: false, count: 0 },
};

/**
 * A connected (logged-in) store state with a basic user and server info.
 */
export const connectedState: Partial<RootState> = {
  ...disconnectedState,
  server: {
    ...(disconnectedState.server as any),
    initialized: true,
    status: {
      connectionAttemptMade: true,
      state: WebsocketTypes.StatusEnum.LOGGED_IN,
      description: null,
    },
    info: {
      message: '<b>Welcome</b>',
      name: 'Test Server',
      version: '1.0.0',
    },
    user: makeUser(),
    users: {
      testUser: makeUser(),
    },
  },
};

/**
 * Connected state with rooms and a joined room containing games and users.
 */
export const connectedWithRoomsState: Partial<RootState> = {
  ...connectedState,
  server: {
    ...(connectedState.server as any),
    users: {
      testUser: makeUser(),
      otherUser: makeUser({ name: 'otherUser' }),
    },
  },
  rooms: {
    ...(disconnectedState.rooms as any),
    rooms: {
      1: {
        info: { roomId: 1, name: 'Main Room', description: 'The main room', autoJoin: true, permissionLevel: 0 },
        gameList: [],
        userList: [makeUser(), makeUser({ name: 'otherUser' })],
      },
    },
    joinedRoomIds: { 1: true },
    messages: {
      1: [],
    },
  },
};

export { makeUser };
