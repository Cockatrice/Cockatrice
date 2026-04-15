import { RoomsDispatch } from '@app/store';
import { Data, Enriched } from '@app/types';

export class RoomPersistence {
  static clearStore() {
    RoomsDispatch.clearStore();
  }

  static joinRoom(roomInfo: Data.ServerInfo_Room) {
    RoomsDispatch.joinRoom(roomInfo);
  }

  static leaveRoom(roomId: number) {
    RoomsDispatch.leaveRoom(roomId);
  }

  static updateRooms(rooms: Data.ServerInfo_Room[]) {
    RoomsDispatch.updateRooms(rooms);
  }

  static updateGames(roomId: number, gameList: Data.ServerInfo_Game[]) {
    RoomsDispatch.updateGames(roomId, gameList);
  }

  static addMessage(roomId: number, message: Enriched.Message) {
    RoomsDispatch.addMessage(roomId, message);
  }

  static userJoined(roomId: number, user: Data.ServerInfo_User) {
    RoomsDispatch.userJoined(roomId, user);
  }

  static userLeft(roomId: number, name: string) {
    RoomsDispatch.userLeft(roomId, name);
  }

  static removeMessages(roomId: number, name: string, amount: number): void {
    RoomsDispatch.removeMessages(roomId, name, amount);
  };

  static gameCreated(roomId: number) {
    RoomsDispatch.gameCreated(roomId);
  }

  static joinedGame(roomId: number, gameId: number) {
    RoomsDispatch.joinedGame(roomId, gameId);
  }
}
