import { User } from "./user";

export interface Room {
  autoJoin: boolean
  description: string;
  gameCount: number;
  gameList: any[];
  gametypeList: any[];
  gametypeMap: { [index: number]: string; };
  name: string;
  permissionlevel: RoomAccessLevel;
  playerCount: number;
  privilegelevel: RoomAccessLevel;
  roomId: number;
  userList: User[];
  order: number;
}

export enum RoomAccessLevel {
  "none"
}
