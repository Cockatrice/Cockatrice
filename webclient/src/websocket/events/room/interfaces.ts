import { Game, User } from 'types';

export interface JoinRoomData {
  userInfo: User;
}

export interface LeaveRoomData {
  name: string;
}

export interface ListGamesData {
  gameList: Game[];
}

export interface RemoveMessagesData {
  name: string;
  amount: number;
}

export interface RoomEvent {
  roomEvent: {
    roomId: number;
  }
}
