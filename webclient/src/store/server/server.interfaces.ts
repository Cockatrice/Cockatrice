import { App, Data, Enriched } from '@app/types';
import { WebsocketTypes } from '@app/websocket/types';

export interface ServerState {
  initialized: boolean;
  /** Buddies keyed by username for O(1) lookup. Use `getSortedBuddyList` for display. */
  buddyList: { [userName: string]: Data.ServerInfo_User };
  /** Ignored users keyed by username for O(1) lookup. Use `getSortedIgnoreList` for display. */
  ignoreList: { [userName: string]: Data.ServerInfo_User };
  info: ServerStateInfo;
  status: ServerStateStatus;
  logs: ServerStateLogs;
  user: Data.ServerInfo_User | null;
  /** Connected users keyed by username for O(1) lookup. Use `getSortedUsers` for display. */
  users: { [userName: string]: Data.ServerInfo_User };
  sortUsersBy: ServerStateSortUsersBy;
  messages: {
    [userName: string]: Data.Event_UserMessage[];
  };
  userInfo: {
    [userName: string]: Data.ServerInfo_User;
  };
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
  /** Replays keyed by gameId for O(1) lookup/update. */
  replays: { [gameId: number]: Data.ServerInfo_ReplayMatch };
  backendDecks: Data.Response_DeckList | null;
  downloadedDeck: { deckId: number; deck: string } | null;
  downloadedReplay: { replayId: number; replayData: Uint8Array } | null;
  gamesOfUser: { [userName: string]: { [gameId: number]: Enriched.Game } };
  registrationError: string | null;
}

export interface ServerStateStatus {
  connectionAttemptMade: boolean;
  description: string | null;
  state: WebsocketTypes.StatusEnum;
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
  field: App.UserSortField;
}
