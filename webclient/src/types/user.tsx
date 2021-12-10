export interface User {
  accountageSecs: number;
  name: string;
  privlevel: UserAccessLevel;
  userLevel: UserPrivLevel;
  realName?: string;
  country?: string;
  avatarBmp?: Uint8Array;
}

export enum UserAccessLevel {
  'NONE'
}

export enum UserPrivLevel {
  'unknown 1',
  'unknown 2',
  'unknown 3',
  'unknown 4'
}

export enum UserSortField {
  NAME = 'name'
}
