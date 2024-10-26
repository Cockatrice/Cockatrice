import { store, RoomsDispatch, RoomsSelectors } from 'store';
import { Game, Message, Room, User } from 'types';
import NormalizeService from '../utils/NormalizeService';

export class RoomPersistence {
  static clearStore() {
    RoomsDispatch.clearStore();
  }

  static joinRoom(roomInfo: Room) {
    NormalizeService.normalizeRoomInfo(roomInfo);
    RoomsDispatch.joinRoom(roomInfo);
  }

  static leaveRoom(roomId: number) {
    RoomsDispatch.leaveRoom(roomId);
  }

  static updateRooms(rooms: Room[]) {
    RoomsDispatch.updateRooms(rooms);
  }

  static updateGames(roomId: number, gameList: Game[]) {
    const game = gameList[0];

    if (!game.gameType) {
      const room = RoomsSelectors.getRoom(store.getState(), roomId);

      if (room) {
        const { gametypeMap } = room;
        NormalizeService.normalizeGameObject(game, gametypeMap);
      }
    }

    RoomsDispatch.updateGames(roomId, gameList);
  }

  static addMessage(roomId: number, message: Message) {
    NormalizeService.normalizeUserMessage(message);

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
