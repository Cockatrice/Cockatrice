import { Log, SortBy, User, UserSortField } from 'types';

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
  buddyList: User[];
  ignoreList: User[];
  info: ServerStateInfo;
  status: ServerStateStatus;
  logs: ServerStateLogs;
  user: User;
  users: User[];
  sortUsersBy: ServerStateSortUsersBy;
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
  room: Log[];
  game: Log[];
  chat: Log[];
}

export interface ServerStateSortUsersBy extends SortBy {
  field: UserSortField
}
