import { App, Data, Enriched } from '@app/types';

export interface ServerState {
  initialized: boolean;
  buddyList: Data.ServerInfo_User[];
  ignoreList: Data.ServerInfo_User[];
  info: ServerStateInfo;
  status: ServerStateStatus;
  logs: ServerStateLogs;
  user: Data.ServerInfo_User | null;
  users: Data.ServerInfo_User[];
  sortUsersBy: ServerStateSortUsersBy;
  messages: {
    [userName: string]: Data.Event_UserMessage[];
  }
  userInfo: {
    [userName: string]: Data.ServerInfo_User;
  }
  notifications: Data.Event_NotifyUser[];
  serverShutdown: Data.Event_ServerShutdown | null;
  banUser: string;
  banHistory: {
    [userName: string]: Data.ServerInfo_Ban[];
  };
  warnHistory: {
    [userName: string]: Data.ServerInfo_Warning[];
  };
  warnListOptions: Data.Response_WarnList[];
  warnUser: string;
  adminNotes: { [userName: string]: string };
  replays: Data.ServerInfo_ReplayMatch[];
  backendDecks: Data.Response_DeckList | null;
  gamesOfUser: { [userName: string]: Enriched.Game[] };
  registrationError: string | null;
}

export interface ServerStateStatus {
  connectionAttemptMade: boolean;
  description: string | null;
  state: App.StatusEnum;
}

export interface ServerStateInfo {
  message: string | null;
  name: string | null;
  version: string | null;
}

export interface ServerStateLogs {
  room: Data.ServerInfo_ChatMessage[];
  game: Data.ServerInfo_ChatMessage[];
  chat: Data.ServerInfo_ChatMessage[];
}

export interface ServerStateSortUsersBy extends App.SortBy {
  field: App.UserSortField
}
