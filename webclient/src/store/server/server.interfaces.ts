import {
  Game, SortBy, UserSortField
} from 'types';
import type { ServerInfo_User } from 'generated/proto/serverinfo_user_pb';
import type { ServerInfo_Ban } from 'generated/proto/serverinfo_ban_pb';
import type { ServerInfo_Warning } from 'generated/proto/serverinfo_warning_pb';
import type { Response_WarnList } from 'generated/proto/response_warn_list_pb';
import type { ServerInfo_ReplayMatch } from 'generated/proto/serverinfo_replay_match_pb';
import type { Response_DeckList } from 'generated/proto/response_deck_list_pb';
import type { ServerInfo_ChatMessage } from 'generated/proto/serverinfo_chat_message_pb';
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
  buddyList: ServerInfo_User[];
  ignoreList: ServerInfo_User[];
  info: ServerStateInfo;
  status: ServerStateStatus;
  logs: ServerStateLogs;
  user: ServerInfo_User;
  users: ServerInfo_User[];
  sortUsersBy: ServerStateSortUsersBy;
  messages: {
    [userName: string]: UserMessageData[];
  }
  userInfo: {
    [userName: string]: ServerInfo_User;
  }
  notifications: NotifyUserData[];
  serverShutdown: ServerShutdownData;
  banUser: string;
  banHistory: {
    [userName: string]: ServerInfo_Ban[];
  };
  warnHistory: {
    [userName: string]: ServerInfo_Warning[];
  };
  warnListOptions: Response_WarnList[];
  warnUser: string;
  adminNotes: { [userName: string]: string };
  replays: ServerInfo_ReplayMatch[];
  backendDecks: Response_DeckList | null;
  gamesOfUser: { [userName: string]: Game[] };
  registrationError: string | null;
}

export interface ServerStateStatus {
  connectionAttemptMade: boolean;
  description: string;
  state: number;
}

export interface ServerStateInfo {
  message: string;
  name: string;
  version: string;
}

export interface ServerStateLogs {
  room: ServerInfo_ChatMessage[];
  game: ServerInfo_ChatMessage[];
  chat: ServerInfo_ChatMessage[];
}

export interface ServerStateSortUsersBy extends SortBy {
  field: UserSortField
}
