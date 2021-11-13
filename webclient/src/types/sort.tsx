export enum SortDirection {
  ASC = 'ASC',
  DESC = 'DESC'
}

export interface SortBy {
  field: string;
  order: SortDirection;
}
