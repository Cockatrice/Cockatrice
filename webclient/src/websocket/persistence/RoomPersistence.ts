import { RoomsDispatch } from 'store';
import { Message, User } from 'types';
import type { ServerInfo_Room } from 'generated/proto/serverinfo_room_pb';
import type { ServerInfo_Game } from 'generated/proto/serverinfo_game_pb';

export class RoomPersistence {
  static clearStore() {
    RoomsDispatch.clearStore();
  }

  static joinRoom(roomInfo: ServerInfo_Room) {
    RoomsDispatch.joinRoom(roomInfo);
  }

  static leaveRoom(roomId: number) {
    RoomsDispatch.leaveRoom(roomId);
  }

  static updateRooms(rooms: ServerInfo_Room[]) {
    RoomsDispatch.updateRooms(rooms);
  }

  static updateGames(roomId: number, gameList: ServerInfo_Game[]) {
    // Guard: the server never sends an empty gameList to signal "clear all games".
    // An empty array here means no game updates — skip the dispatch to avoid
    // unnecessarily overwriting the existing game list with an empty one.
    if (!gameList?.length) {
      return;
    }

    RoomsDispatch.updateGames(roomId, gameList);
  }

  static addMessage(roomId: number, message: Message) {
    RoomsDispatch.addMessage(roomId, message);
  }

  static userJoined(roomId: number, user: User) {
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
