export enum SortDirection {
  ASC = 'ASC',
  DESC = 'DESC'
}

export interface SortBy<T extends string = string> {
  field: T;
  order: SortDirection;
}

export enum GameSortField {
  // Nested under `info` because Enriched.Game is a composition type.
  START_TIME = 'info.startTime'
}

export enum UserSortField {
  NAME = 'name'
}
