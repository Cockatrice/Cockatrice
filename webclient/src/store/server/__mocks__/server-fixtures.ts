import { App, Data, Enriched } from '@app/types';
import type { MessageInitShape } from '@bufbuild/protobuf';

import { create } from '@bufbuild/protobuf';
import { ServerState } from '../server.interfaces';

export function makeUser(
  overrides: MessageInitShape<typeof Data.ServerInfo_UserSchema> = {}
): Data.ServerInfo_User {
  return create(Data.ServerInfo_UserSchema, {
    name: 'TestUser',
    accountageSecs: 0n,
    privlevel: '',
    userLevel: 0,
    ...overrides,
  });
}

export function makeLogItem(
  overrides: MessageInitShape<typeof Data.ServerInfo_ChatMessageSchema> = {}
): Data.ServerInfo_ChatMessage {
  return create(Data.ServerInfo_ChatMessageSchema, {
    message: '',
    senderId: '',
    senderIp: '',
    senderName: '',
    targetId: '',
    targetName: '',
    targetType: '',
    time: '',
    ...overrides,
  });
}

export function makeBanHistoryItem(
  overrides: MessageInitShape<typeof Data.ServerInfo_BanSchema> = {}
): Data.ServerInfo_Ban {
  return create(Data.ServerInfo_BanSchema, {
    adminId: '',
    adminName: '',
    banTime: '',
    banLength: '',
    banReason: '',
    visibleReason: '',
    ...overrides,
  });
}

export function makeWarnHistoryItem(
  overrides: MessageInitShape<typeof Data.ServerInfo_WarningSchema> = {}
): Data.ServerInfo_Warning {
  return create(Data.ServerInfo_WarningSchema, {
    userName: '',
    adminName: '',
    reason: '',
    timeOf: '',
    ...overrides,
  });
}

export function makeWarnListItem(
  overrides: MessageInitShape<typeof Data.Response_WarnListSchema> = {}
): Data.Response_WarnList {
  return create(Data.Response_WarnListSchema, {
    warning: [],
    userName: '',
    userClientid: '',
    ...overrides,
  });
}

export function makeDeckTreeItem(
  overrides: MessageInitShape<typeof Data.ServerInfo_DeckStorage_TreeItemSchema> = {},
): Data.ServerInfo_DeckStorage_TreeItem {
  return create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
    id: 1,
    name: 'item',
    ...overrides,
  });
}

export function makeDeckList(
  overrides: MessageInitShape<typeof Data.Response_DeckListSchema> = {}
): Data.Response_DeckList {
  return create(Data.Response_DeckListSchema, {
    root: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] }),
    ...overrides,
  });
}

export function makeReplayMatch(
  overrides: MessageInitShape<typeof Data.ServerInfo_ReplayMatchSchema> = {}
): Data.ServerInfo_ReplayMatch {
  return create(Data.ServerInfo_ReplayMatchSchema, {
    gameId: 1,
    roomName: 'Test Room',
    timeStarted: 0,
    length: 0,
    gameName: 'Test Game',
    playerNames: [],
    doNotHide: false,
    replayList: [],
    ...overrides,
  });
}

type MakeGameOverrides = MessageInitShape<typeof Data.ServerInfo_GameSchema> & {
  gameType?: string;
};

export function makeGame(overrides: MakeGameOverrides = {}): Enriched.Game {
  const { gameType = '', ...protoFields } = overrides;
  return {
    info: create(Data.ServerInfo_GameSchema, { description: '', ...protoFields }),
    gameType,
  };
}

export function makeLoginSuccessContext(
  overrides: Partial<Enriched.LoginSuccessContext> = {}
): Enriched.LoginSuccessContext {
  return {
    hashedPassword: 'hash',
    ...overrides,
  };
}

export function makePendingActivationContext(
  overrides: Partial<Enriched.PendingActivationContext> = {}
): Enriched.PendingActivationContext {
  return {
    host: 'localhost',
    port: '4747',
    userName: 'user',
    ...overrides,
  };
}

export function makeServerState(overrides: Partial<ServerState> = {}): ServerState {
  return {
    initialized: false,
    buddyList: {},
    ignoreList: {},
    status: {
      connectionAttemptMade: false,
      state: App.StatusEnum.DISCONNECTED,
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
    users: {},
    sortUsersBy: {
      field: App.UserSortField.NAME,
      order: App.SortDirection.ASC,
    },
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
    ...overrides,
  };
}
