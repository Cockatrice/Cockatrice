import {
  BanHistoryItem,
  DeckList,
  DeckStorageTreeItem,
  LogItem,
  ReplayMatch,
  SortDirection,
  StatusEnum,
  User,
  UserPrivLevel,
  UserSortField,
  WebSocketConnectOptions,
  WarnHistoryItem,
  WarnListItem,
} from 'types';
import { ServerState } from '../server.interfaces';

export function makeUser(overrides: Partial<User> = {}): User {
  return {
    name: 'TestUser',
    accountageSecs: 0,
    privlevel: UserPrivLevel.NONE,
    userLevel: 0,
    ...overrides,
  };
}

export function makeLogItem(overrides: Partial<LogItem> = {}): LogItem {
  return {
    message: '',
    senderId: '',
    senderIp: '',
    senderName: '',
    targetId: '',
    targetName: '',
    targetType: '',
    time: '',
    ...overrides,
  };
}

export function makeBanHistoryItem(overrides: Partial<BanHistoryItem> = {}): BanHistoryItem {
  return {
    adminId: '',
    adminName: '',
    banTime: '',
    banLength: '',
    banReason: '',
    visibleReason: '',
    ...overrides,
  };
}

export function makeWarnHistoryItem(overrides: Partial<WarnHistoryItem> = {}): WarnHistoryItem {
  return {
    userName: '',
    adminName: '',
    reason: '',
    timeOf: '',
    ...overrides,
  };
}

export function makeWarnListItem(overrides: Partial<WarnListItem> = {}): WarnListItem {
  return {
    warning: '',
    userName: '',
    userClientid: '',
    ...overrides,
  };
}

export function makeDeckTreeItem(overrides: Partial<DeckStorageTreeItem> = {}): DeckStorageTreeItem {
  return {
    id: 1,
    name: 'item',
    file: { creationTime: 0 },
    folder: null,
    ...overrides,
  };
}

export function makeDeckList(overrides: Partial<DeckList> = {}): DeckList {
  return {
    root: { items: [] },
    ...overrides,
  };
}

export function makeReplayMatch(overrides: Partial<ReplayMatch> = {}): ReplayMatch {
  return {
    gameId: 1,
    roomName: 'Test Room',
    timeStarted: 0,
    length: 0,
    gameName: 'Test Game',
    playerNames: [],
    doNotHide: false,
    replayList: [],
    ...overrides,
  };
}

export function makeConnectOptions(overrides: Partial<WebSocketConnectOptions> = {}): WebSocketConnectOptions {
  return {
    host: 'localhost',
    port: '4747',
    userName: 'user',
    password: 'pass',
    ...overrides,
  };
}

export function makeServerState(overrides: Partial<ServerState> = {}): ServerState {
  return {
    initialized: false,
    buddyList: [],
    ignoreList: [],
    status: {
      state: StatusEnum.DISCONNECTED,
      description: null,
    },
    info: {
      message: null,
      name: null,
      version: null,
    },
    logs: {
      room: [],
      game: [],
      chat: [],
    },
    user: null,
    users: [],
    sortUsersBy: {
      field: UserSortField.NAME,
      order: SortDirection.ASC,
    },
    connectOptions: {},
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
    replays: [],
    backendDecks: null,
    gamesOfUser: {},
    ...overrides,
  };
}
