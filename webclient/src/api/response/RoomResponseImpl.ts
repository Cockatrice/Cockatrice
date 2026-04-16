import { Data } from '@app/types';
import type { IRoomResponse, WebSocketRoomResponseOverrides } from '@app/websocket';
import { RoomsDispatch } from '@app/store';

type Message = WebSocketRoomResponseOverrides['Event_RoomSay'];

export class RoomResponseImpl implements IRoomResponse<WebSocketRoomResponseOverrides> {
  clearStore(): void {
    RoomsDispatch.clearStore();
  }

  joinRoom(roomInfo: Data.ServerInfo_Room): void {
    RoomsDispatch.joinRoom(roomInfo);
  }

  leaveRoom(roomId: number): void {
    RoomsDispatch.leaveRoom(roomId);
  }

  updateRooms(rooms: Data.ServerInfo_Room[]): void {
    RoomsDispatch.updateRooms(rooms);
  }

  updateGames(roomId: number, gameList: Data.ServerInfo_Game[]): void {
    RoomsDispatch.updateGames(roomId, gameList);
  }

  addMessage(roomId: number, message: Message): void {
    RoomsDispatch.addMessage(roomId, message);
  }

  userJoined(roomId: number, user: Data.ServerInfo_User): void {
    RoomsDispatch.userJoined(roomId, user);
  }

  userLeft(roomId: number, name: string): void {
    RoomsDispatch.userLeft(roomId, name);
  }

  removeMessages(roomId: number, name: string, amount: number): void {
    RoomsDispatch.removeMessages(roomId, name, amount);
  }

  gameCreated(roomId: number): void {
    RoomsDispatch.gameCreated(roomId);
  }

  joinedGame(roomId: number, gameId: number): void {
    RoomsDispatch.joinedGame(roomId, gameId);
  }
}
