export enum SortDirection {
  ASC = 'ASC',
  DESC = 'DESC'
}

export interface SortBy<T extends string = string> {
  field: T;
  order: SortDirection;
}

export enum GameSortField {
  START_TIME = 'startTime'
}

export enum UserSortField {
  NAME = 'name'
}
