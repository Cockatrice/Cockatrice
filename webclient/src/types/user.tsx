export interface User {
  accountageSecs: number;
  avatarBmp: Uint8Array;
  country: string;
  gender: number;
  name: string;
  privlevel: UserAccessLevel;
  realName: string;
  userLevel: UserPrivLevel;
}

export enum UserAccessLevel {
  "NONE"
}

export enum UserPrivLevel {
  "unknown 1",
  "unknown 2",
  "unknown 3",
  "unknown 4"
}

export enum UserSortField {
  NAME = "name"
}