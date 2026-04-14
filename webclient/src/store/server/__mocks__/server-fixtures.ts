import {
  Game,
  ProtoInit,
  SortDirection,
  StatusEnum,
  UserSortField,
  WebSocketConnectOptions,
} from 'types';
import type { ServerInfo_User } from 'generated/proto/serverinfo_user_pb';
import type { ServerInfo_Ban } from 'generated/proto/serverinfo_ban_pb';
import type { ServerInfo_Warning } from 'generated/proto/serverinfo_warning_pb';
import type { Response_WarnList } from 'generated/proto/response_warn_list_pb';
import type { ServerInfo_ReplayMatch } from 'generated/proto/serverinfo_replay_match_pb';
import type { ServerInfo_ChatMessage } from 'generated/proto/serverinfo_chat_message_pb';
import type { Response_DeckList } from 'generated/proto/response_deck_list_pb';
import type { ServerInfo_DeckStorage_TreeItem } from 'generated/proto/serverinfo_deckstorage_pb';
import { create } from '@bufbuild/protobuf';
import { ServerInfo_GameSchema } from 'generated/proto/serverinfo_game_pb';
import { ServerInfo_UserSchema } from 'generated/proto/serverinfo_user_pb';
import { ServerInfo_ReplayMatchSchema } from 'generated/proto/serverinfo_replay_match_pb';
import { ServerInfo_ChatMessageSchema } from 'generated/proto/serverinfo_chat_message_pb';
import { ServerInfo_BanSchema } from 'generated/proto/serverinfo_ban_pb';
import { ServerInfo_WarningSchema } from 'generated/proto/serverinfo_warning_pb';
import { Response_WarnListSchema } from 'generated/proto/response_warn_list_pb';
import { ServerInfo_DeckStorage_TreeItemSchema, ServerInfo_DeckStorage_FolderSchema } from 'generated/proto/serverinfo_deckstorage_pb';
import { Response_DeckListSchema } from 'generated/proto/response_deck_list_pb';
import { ServerState } from '../server.interfaces';

export function makeUser(overrides: ProtoInit<ServerInfo_User> = {}): ServerInfo_User {
  return create(ServerInfo_UserSchema, {
    name: 'TestUser',
    accountageSecs: 0n,
    privlevel: '',
    userLevel: 0,
    ...overrides,
  });
}

export function makeLogItem(overrides: ProtoInit<ServerInfo_ChatMessage> = {}): ServerInfo_ChatMessage {
  return create(ServerInfo_ChatMessageSchema, {
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

export function makeBanHistoryItem(overrides: ProtoInit<ServerInfo_Ban> = {}): ServerInfo_Ban {
  return create(ServerInfo_BanSchema, {
    adminId: '',
    adminName: '',
    banTime: '',
    banLength: '',
    banReason: '',
    visibleReason: '',
    ...overrides,
  });
}

export function makeWarnHistoryItem(overrides: ProtoInit<ServerInfo_Warning> = {}): ServerInfo_Warning {
  return create(ServerInfo_WarningSchema, {
    userName: '',
    adminName: '',
    reason: '',
    timeOf: '',
    ...overrides,
  });
}

export function makeWarnListItem(overrides: ProtoInit<Response_WarnList> = {}): Response_WarnList {
  return create(Response_WarnListSchema, {
    warning: [],
    userName: '',
    userClientid: '',
    ...overrides,
  });
}

export function makeDeckTreeItem(overrides: ProtoInit<ServerInfo_DeckStorage_TreeItem> = {}): ServerInfo_DeckStorage_TreeItem {
  return create(ServerInfo_DeckStorage_TreeItemSchema, {
    id: 1,
    name: 'item',
    ...overrides,
  });
}

export function makeDeckList(overrides: ProtoInit<Response_DeckList> = {}): Response_DeckList {
  return create(Response_DeckListSchema, {
    root: create(ServerInfo_DeckStorage_FolderSchema, { items: [] }),
    ...overrides,
  });
}

export function makeReplayMatch(overrides: ProtoInit<ServerInfo_ReplayMatch> = {}): ServerInfo_ReplayMatch {
  return create(ServerInfo_ReplayMatchSchema, {
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

export function makeGame(overrides: Partial<Game> = {}): Game {
  return { ...create(ServerInfo_GameSchema, { description: '' }), gameType: '', ...overrides };
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
      connectionAttemptMade: false,
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
    registrationError: null,
    ...overrides,
  };
}
