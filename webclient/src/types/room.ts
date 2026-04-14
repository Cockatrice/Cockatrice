import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { Game } from './game';

export interface GametypeMap { [index: number]: string }

export type Room = ServerInfo_Room & {
  gametypeMap: GametypeMap;
  gameList: Game[];
  order: number;
};
