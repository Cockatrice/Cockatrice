import { createSlice, PayloadAction } from '@reduxjs/toolkit';
import { App, Data, Enriched } from '@app/types';
import { create } from '@bufbuild/protobuf';

import { normalizeBannedUserError, normalizeGameObject, normalizeGametypeMap, normalizeLogs } from '../common';

import { ServerState, ServerStateStatus } from './server.interfaces';

function splitPath(path: string): string[] {
  return path ? path.split('/') : [];
}

function insertAtPath(
  folder: Data.ServerInfo_DeckStorage_Folder,
  pathSegments: string[],
  item: Data.ServerInfo_DeckStorage_TreeItem,
): Data.ServerInfo_DeckStorage_Folder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [...folder.items, item] });
  }
  const [head, ...tail] = pathSegments;
  const match = folder.items.find(child => child.name === head && child.folder);
  if (match) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, {
      items: folder.items.map(child =>
        child === match
          ? { ...child, folder: insertAtPath(child.folder!, tail, item) }
          : child
      ),
    });
  }
  const created: Data.ServerInfo_DeckStorage_TreeItem = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
    id: 0, name: head, folder: insertAtPath(create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] }), tail, item)
  });
  return create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [...folder.items, created] });
}

function removeById(folder: Data.ServerInfo_DeckStorage_Folder, id: number): Data.ServerInfo_DeckStorage_Folder {
  return create(Data.ServerInfo_DeckStorage_FolderSchema, {
    items: folder.items
      .filter(item => item.id !== id)
      .map(item =>
        item.folder ? { ...item, folder: removeById(item.folder, id) } : item
      ),
  });
}

function removeByPath(folder: Data.ServerInfo_DeckStorage_Folder, pathSegments: string[]): Data.ServerInfo_DeckStorage_Folder {
  if (pathSegments.length === 0 || (pathSegments.length === 1 && pathSegments[0] === '')) {
    return folder;
  }
  const [head, ...tail] = pathSegments;
  if (tail.length === 0) {
    return create(Data.ServerInfo_DeckStorage_FolderSchema, {
      items: folder.items.filter(item => !(item.name === head && item.folder != null))
    });
  }
  return create(Data.ServerInfo_DeckStorage_FolderSchema, {
    items: folder.items.map(item =>
      item.name === head && item.folder
        ? { ...item, folder: removeByPath(item.folder, tail) }
        : item
    ),
  });
}

const initialState: ServerState = {
  initialized: false,
  buddyList: {},
  ignoreList: {},

  status: {
    connectionAttemptMade: false,
    state: App.StatusEnum.DISCONNECTED,
    description: null
  },
  info: {
    message: null,
    name: null,
    version: null
  },
  logs: {
    room: [],
    game: [],
    chat: []
  },
  user: null,
  users: {},
  sortUsersBy: {
    field: App.UserSortField.NAME,
    order: App.SortDirection.ASC
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
};

export const serverSlice = createSlice({
  name: 'server',
  initialState,
  reducers: {
    initialized: () => ({
      ...initialState,
      initialized: true,
    }),

    connectionAttempted: (state) => {
      state.status.connectionAttemptMade = true;
    },

    clearStore: (state) => ({
      ...initialState,
      status: { ...state.status },
    }),

    serverMessage: (state, action: PayloadAction<{ message: string }>) => {
      state.info.message = action.payload.message;
    },

    updateBuddyList: (state, action: PayloadAction<{ buddyList: Data.ServerInfo_User[] }>) => {
      const buddyList: { [userName: string]: Data.ServerInfo_User } = {};
      for (const user of action.payload.buddyList) {
        buddyList[user.name] = user;
      }
      state.buddyList = buddyList;
    },

    addToBuddyList: (state, action: PayloadAction<{ user: Data.ServerInfo_User }>) => {
      const { user } = action.payload;
      state.buddyList[user.name] = user;
    },

    removeFromBuddyList: (state, action: PayloadAction<{ userName: string }>) => {
      delete state.buddyList[action.payload.userName];
    },

    updateIgnoreList: (state, action: PayloadAction<{ ignoreList: Data.ServerInfo_User[] }>) => {
      const ignoreList: { [userName: string]: Data.ServerInfo_User } = {};
      for (const user of action.payload.ignoreList) {
        ignoreList[user.name] = user;
      }
      state.ignoreList = ignoreList;
    },

    addToIgnoreList: (state, action: PayloadAction<{ user: Data.ServerInfo_User }>) => {
      const { user } = action.payload;
      state.ignoreList[user.name] = user;
    },

    removeFromIgnoreList: (state, action: PayloadAction<{ userName: string }>) => {
      delete state.ignoreList[action.payload.userName];
    },

    updateInfo: (state, action: PayloadAction<{ info: { name: string; version: string } }>) => {
      const { name, version } = action.payload.info;
      state.info.name = name;
      state.info.version = version;
    },

    updateStatus: (state, action: PayloadAction<{ status: Pick<ServerStateStatus, 'state' | 'description'> }>) => {
      const { status } = action.payload;
      state.status = { ...state.status, ...status };

      if (status.state === App.StatusEnum.DISCONNECTED) {
        state.status.connectionAttemptMade = false;
      }
    },

    updateUser: (state, action: PayloadAction<{ user: Partial<Data.ServerInfo_User> }>) => {
      state.user = state.user
        ? { ...state.user, ...action.payload.user } as Data.ServerInfo_User
        : action.payload.user as Data.ServerInfo_User;
    },

    updateUsers: (state, action: PayloadAction<{ users: Data.ServerInfo_User[] }>) => {
      const users: { [userName: string]: Data.ServerInfo_User } = {};
      for (const user of action.payload.users) {
        users[user.name] = user;
      }
      state.users = users;
    },

    userJoined: (state, action: PayloadAction<{ user: Data.ServerInfo_User }>) => {
      const { user } = action.payload;
      state.users[user.name] = user;
    },

    userLeft: (state, action: PayloadAction<{ name: string }>) => {
      delete state.users[action.payload.name];
    },

    viewLogs: (state, action: PayloadAction<{ logs: Data.ServerInfo_ChatMessage[] }>) => {
      state.logs = { ...normalizeLogs(action.payload.logs) };
    },

    clearLogs: (state) => {
      state.logs = { ...initialState.logs };
    },

    userMessage: (state, action: PayloadAction<{ messageData: Data.Event_UserMessage }>) => {
      const { senderName, receiverName } = action.payload.messageData;
      const userName = state.user!.name === senderName ? receiverName : senderName;
      if (!state.messages[userName]) {
        state.messages[userName] = [];
      }
      state.messages[userName].push(action.payload.messageData);
    },

    getUserInfo: (state, action: PayloadAction<{ userInfo: Data.ServerInfo_User }>) => {
      const { userInfo } = action.payload;
      state.userInfo[userInfo.name] = userInfo;
    },

    notifyUser: (state, action: PayloadAction<{ notification: Data.Event_NotifyUser }>) => {
      state.notifications.push(action.payload.notification);
    },

    serverShutdown: (state, action: PayloadAction<{ data: Data.Event_ServerShutdown }>) => {
      state.serverShutdown = action.payload.data;
    },

    banFromServer: (state, action: PayloadAction<{ userName: string }>) => {
      state.banUser = action.payload.userName;
    },

    banHistory: (state, action: PayloadAction<{ userName: string; banHistory: Data.ServerInfo_Ban[] }>) => {
      state.banHistory[action.payload.userName] = action.payload.banHistory;
    },

    warnHistory: (state, action: PayloadAction<{ userName: string; warnHistory: Data.ServerInfo_Warning[] }>) => {
      state.warnHistory[action.payload.userName] = action.payload.warnHistory;
    },

    warnListOptions: (state, action: PayloadAction<{ warnList: Data.Response_WarnList[] }>) => {
      state.warnListOptions = action.payload.warnList;
    },

    warnUser: (state, action: PayloadAction<{ userName: string }>) => {
      state.warnUser = action.payload.userName;
    },

    getAdminNotes: (state, action: PayloadAction<{ userName: string; notes: string }>) => {
      state.adminNotes[action.payload.userName] = action.payload.notes;
    },

    updateAdminNotes: (state, action: PayloadAction<{ userName: string; notes: string }>) => {
      state.adminNotes[action.payload.userName] = action.payload.notes;
    },

    adjustMod: (state, action: PayloadAction<{ userName: string; shouldBeMod: boolean; shouldBeJudge: boolean }>) => {
      const { userName, shouldBeMod, shouldBeJudge } = action.payload;
      const user = state.users[userName];
      if (!user) {
        return;
      }
      let newLevel = user.userLevel;
      newLevel = shouldBeMod
        ? (newLevel | Data.ServerInfo_User_UserLevelFlag.IsModerator)
        : (newLevel & ~Data.ServerInfo_User_UserLevelFlag.IsModerator);
      newLevel = shouldBeJudge
        ? (newLevel | Data.ServerInfo_User_UserLevelFlag.IsJudge)
        : (newLevel & ~Data.ServerInfo_User_UserLevelFlag.IsJudge);
      state.users[userName] = { ...user, userLevel: newLevel };
    },

    replayList: (state, action: PayloadAction<{ matchList: Data.ServerInfo_ReplayMatch[] }>) => {
      const replays: { [gameId: number]: Data.ServerInfo_ReplayMatch } = {};
      for (const match of action.payload.matchList) {
        replays[match.gameId] = match;
      }
      state.replays = replays;
    },

    replayAdded: (state, action: PayloadAction<{ matchInfo: Data.ServerInfo_ReplayMatch }>) => {
      const { matchInfo } = action.payload;
      state.replays[matchInfo.gameId] = matchInfo;
    },

    replayModifyMatch: (state, action: PayloadAction<{ gameId: number; doNotHide: boolean }>) => {
      const { gameId, doNotHide } = action.payload;
      const existing = state.replays[gameId];
      if (!existing) {
        return;
      }
      state.replays[gameId] = { ...existing, doNotHide };
    },

    replayDeleteMatch: (state, action: PayloadAction<{ gameId: number }>) => {
      delete state.replays[action.payload.gameId];
    },

    backendDecks: (state, action: PayloadAction<{ deckList: Data.Response_DeckList }>) => {
      state.backendDecks = action.payload.deckList;
    },

    deckUpload: (state, action: PayloadAction<{ path: string; treeItem: Data.ServerInfo_DeckStorage_TreeItem }>) => {
      if (!state.backendDecks?.root) {
        return;
      }
      state.backendDecks = create(Data.Response_DeckListSchema, {
        root: insertAtPath(state.backendDecks.root, splitPath(action.payload.path), action.payload.treeItem),
      });
    },

    deckDelete: (state, action: PayloadAction<{ deckId: number }>) => {
      if (!state.backendDecks?.root) {
        return;
      }
      state.backendDecks = create(Data.Response_DeckListSchema, {
        root: removeById(state.backendDecks.root, action.payload.deckId),
      });
    },

    deckNewDir: (state, action: PayloadAction<{ path: string; dirName: string }>) => {
      if (!state.backendDecks?.root) {
        return;
      }
      const newFolder: Data.ServerInfo_DeckStorage_TreeItem = create(Data.ServerInfo_DeckStorage_TreeItemSchema, {
        id: 0, name: action.payload.dirName, folder: create(Data.ServerInfo_DeckStorage_FolderSchema, { items: [] })
      });
      state.backendDecks = create(Data.Response_DeckListSchema, {
        root: insertAtPath(state.backendDecks.root, splitPath(action.payload.path), newFolder),
      });
    },

    deckDelDir: (state, action: PayloadAction<{ path: string }>) => {
      if (!state.backendDecks?.root) {
        return;
      }
      state.backendDecks = create(Data.Response_DeckListSchema, {
        root: removeByPath(state.backendDecks.root, splitPath(action.payload.path)),
      });
    },

    deckDownloaded: (state, action: PayloadAction<{ deckId: number; deck: string }>) => {
      state.downloadedDeck = action.payload;
    },

    replayDownloaded: (state, action: PayloadAction<{ replayId: number; replayData: Uint8Array }>) => {
      state.downloadedReplay = action.payload;
    },

    gamesOfUser: (state, action: PayloadAction<{ userName: string; response: Data.Response_GetGamesOfUser }>) => {
      const { userName, response } = action.payload;
      const gametypeMap = normalizeGametypeMap(
        (response.roomList ?? []).flatMap(room => room.gametypeList ?? [])
      );
      const games: { [gameId: number]: Enriched.Game } = {};
      for (const g of response.gameList ?? []) {
        const normalized = normalizeGameObject(g, gametypeMap);
        games[normalized.info.gameId] = normalized;
      }
      state.gamesOfUser[userName] = games;
    },

    registrationFailed: (state, action: PayloadAction<{ reason: string; endTime?: number }>) => {
      const { reason, endTime } = action.payload;
      const error = endTime
        ? normalizeBannedUserError(reason, endTime)
        : reason;
      state.registrationError = error;
    },

    clearRegistrationErrors: (state) => {
      state.registrationError = null;
    },

    accountEditChanged: (state, action: PayloadAction<{ user: Partial<Data.ServerInfo_User> }>) => {
      state.user = { ...state.user, ...action.payload.user } as Data.ServerInfo_User;
    },

    accountImageChanged: (state, action: PayloadAction<{ user: Partial<Data.ServerInfo_User> }>) => {
      state.user = { ...state.user, ...action.payload.user } as Data.ServerInfo_User;
    },

    // Signal-only action types — no state mutation, defined so type strings are generated
    accountAwaitingActivation: (_state, _action: PayloadAction<any>) => {},
    accountActivationFailed: (_state, _action: PayloadAction<any>) => {},
    accountActivationSuccess: (_state, _action: PayloadAction<any>) => {},
    loginSuccessful: (_state, _action: PayloadAction<any>) => {},
    loginFailed: (_state, _action: PayloadAction<any>) => {},
    connectionFailed: (_state, _action: PayloadAction<any>) => {},
    testConnectionSuccessful: (_state, _action: PayloadAction<any>) => {},
    testConnectionFailed: (_state, _action: PayloadAction<any>) => {},
    registrationRequiresEmail: (_state, _action: PayloadAction<any>) => {},
    registrationSuccess: (_state, _action: PayloadAction<any>) => {},
    registrationEmailError: (_state, _action: PayloadAction<any>) => {},
    registrationPasswordError: (_state, _action: PayloadAction<any>) => {},
    registrationUserNameError: (_state, _action: PayloadAction<any>) => {},
    resetPassword: (_state, _action: PayloadAction<any>) => {},
    resetPasswordFailed: (_state, _action: PayloadAction<any>) => {},
    resetPasswordChallenge: (_state, _action: PayloadAction<any>) => {},
    resetPasswordSuccess: (_state, _action: PayloadAction<any>) => {},
    reloadConfig: (_state, _action: PayloadAction<any>) => {},
    shutdownServer: (_state, _action: PayloadAction<any>) => {},
    updateServerMessage: (_state, _action: PayloadAction<any>) => {},
    accountPasswordChange: (_state, _action: PayloadAction<any>) => {},
    addToList: (_state, _action: PayloadAction<any>) => {},
    removeFromList: (_state, _action: PayloadAction<any>) => {},
    grantReplayAccess: (_state, _action: PayloadAction<any>) => {},
    forceActivateUser: (_state, _action: PayloadAction<any>) => {},
  },
});

export const serverReducer = serverSlice.reducer;
