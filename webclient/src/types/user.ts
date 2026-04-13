import type { ServerInfo_User } from 'generated/proto/serverinfo_user_pb';

export type User = ServerInfo_User;
export { ServerInfo_User_UserLevelFlag as UserLevelFlag } from 'generated/proto/serverinfo_user_pb';

export enum UserSortField {
  NAME = 'name'
}
