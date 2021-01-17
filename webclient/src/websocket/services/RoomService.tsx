import { store, RoomsDispatch, RoomsSelectors } from "store";
import { WebClient } from "../WebClient"; 

import { NormalizeService } from "websocket";

export default class RoomService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  clearStore() {
    RoomsDispatch.clearStore();
  }

  joinRoom(roomInfo) {
    NormalizeService.normalizeRoomInfo(roomInfo);
    RoomsDispatch.joinRoom(roomInfo);
  }

  updateRooms(rooms) {
    RoomsDispatch.updateRooms(rooms);
  }

  updateGames(roomId, gameList) {
    const game = gameList[0];

    if (!game.gameType) {
      const { gametypeMap } = RoomsSelectors.getRoom(store.getState(), roomId);
      NormalizeService.normalizeGameObject(game, gametypeMap);
    }

    RoomsDispatch.updateGames(roomId, gameList);
  }

  addMessage(roomId, message) {
    NormalizeService.normalizeUserMessage(message);

    RoomsDispatch.addMessage(roomId, message);
  }

  userJoined(roomId, user) {
    RoomsDispatch.userJoined(roomId, user);
  }

  userLeft(roomId, name) {
    RoomsDispatch.userLeft(roomId, name);
  }
}
