import { Game, User } from 'types';

export interface RoomEvent {
  roomEvent: {
    roomId: number;
  }
}

export interface JoinRoomData {
  userInfo: User;
}

export interface LeaveRoomData {
  name: string;
}

export interface ListGamesData {
  gameList: Game[];
}
