export interface User {
  accountageSecs: number;
  name: string;
  privlevel: UserPrivLevel;
  userLevel: number;
  realName?: string;
  country?: string;
  avatarBmp?: Uint8Array;
}

export enum UserLevelFlag {
  IsNothing = 0,
  IsUser = 1,
  IsRegistered = 2,
  IsModerator = 4,
  IsAdmin = 8,
  IsJudge = 16,
}

export enum UserPrivLevel {
  NONE = 0,
  VIP = 1,
  DONOR = 2
}

export enum UserSortField {
  NAME = 'name'
}
