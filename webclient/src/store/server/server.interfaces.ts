import { WarnHistoryItem, BanHistoryItem, LogItem, SortBy, User, UserSortField, WebSocketConnectOptions, WarnListItem } from 'types';
import { NotifyUserData, ServerShutdownData, UserMessageData } from 'websocket/events/session/interfaces';

export interface ServerConnectParams {
  host: string;
  port: string;
  userName: string;
  password: string;
}

export interface ServerRegisterParams {
  host: string;
  port: string;
  userName: string;
  password: string;
  email: string;
  country: string;
  realName: string;
}

export interface RequestPasswordSaltParams {
  userName: string;
}

export interface ForgotPasswordParams {
  userName: string;
}

export interface ForgotPasswordChallengeParams extends ForgotPasswordParams {
  email: string;
}

export interface ForgotPasswordResetParams extends ForgotPasswordParams {
  token: string;
  newPassword: string;
}

export interface AccountActivationParams extends ServerRegisterParams {
  token: string;
}

export interface ServerState {
  initialized: boolean;
  buddyList: User[];
  ignoreList: User[];
  info: ServerStateInfo;
  status: ServerStateStatus;
  logs: ServerStateLogs;
  user: User;
  users: User[];
  sortUsersBy: ServerStateSortUsersBy;
  connectOptions: WebSocketConnectOptions;
  messages: {
    [userName: string]: UserMessageData[];
  }
  userInfo: {
    [userName: string]: User;
  }
  notifications: NotifyUserData[];
  serverShutdown: ServerShutdownData;
  banUser: string;
  banHistory: {
    [userName: string]: BanHistoryItem[];
  };
  warnHistory: {
    [userName: string]: WarnHistoryItem[];
  };
  warnListOptions: WarnListItem[];
  warnUser: string;
}

export interface ServerStateStatus {
  description: string;
  state: number;
}

export interface ServerStateInfo {
  message: string;
  name: string;
  version: string;
}

export interface ServerStateLogs {
  room: LogItem[];
  game: LogItem[];
  chat: LogItem[];
}

export interface ServerStateSortUsersBy extends SortBy {
  field: UserSortField
}
