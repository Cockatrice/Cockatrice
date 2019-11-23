import { SortBy, User, UserSortField } from 'types';

export interface ServerConnectParams {
  host: string;
  port: string;
  user: string;
  pass: string;
}

export interface ServerState {
  info: ServerStateInfo;
  status: ServerStateStatus;
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

export interface ServerStateSortUsersBy extends SortBy {
  field: UserSortField
}