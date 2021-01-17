import { Log, SortBy, User, UserSortField } from "types";

export interface ServerConnectParams {
  host: string;
  port: string;
  user: string;
  pass: string;
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