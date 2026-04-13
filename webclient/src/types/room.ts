import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';

export interface GametypeMap { [index: number]: string }

export type Room = ServerInfo_Room & {
  gametypeMap: GametypeMap;
  order: number;
};
