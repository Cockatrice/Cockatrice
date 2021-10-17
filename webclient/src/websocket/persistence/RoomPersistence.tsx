import { store, RoomsDispatch, RoomsSelectors } from "store";
import NormalizeService from "../utils/NormalizeService";

export class RoomPersistence {
  static clearStore() {
    RoomsDispatch.clearStore();
  }

  static joinRoom(roomInfo) {
    NormalizeService.normalizeRoomInfo(roomInfo);
    RoomsDispatch.joinRoom(roomInfo);
  }

  static leaveRoom(roomId) {
    RoomsDispatch.leaveRoom(roomId);
  }

  static updateRooms(rooms) {
    RoomsDispatch.updateRooms(rooms);
  }

  static updateGames(roomId, gameList) {
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

  static addMessage(roomId, message) {
    NormalizeService.normalizeUserMessage(message);

    RoomsDispatch.addMessage(roomId, message);
  }

  static userJoined(roomId, user) {
    RoomsDispatch.userJoined(roomId, user);
  }

  static userLeft(roomId, name) {
    RoomsDispatch.userLeft(roomId, name);
  }
}
