import { Dispatch, Selectors } from "store/rooms";
import { store } from "store";


import { WebClient } from "../WebClient";

import { NormalizeService } from "./NormalizeService";

export class RoomService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  clearStore() {
    Dispatch.clearStore();
  }

  joinRoom(roomInfo) {
    NormalizeService.normalizeRoomInfo(roomInfo);
    Dispatch.joinRoom(roomInfo);
  }

  updateRooms(rooms) {
    Dispatch.updateRooms(rooms);
  }

  updateGames(roomId, gameList) {
    const game = gameList[0];

    if (!game.gameType) {
      const { gametypeMap } = Selectors.getRoom(store.getState(), roomId);
      NormalizeService.normalizeGameObject(game, gametypeMap);
    }

    Dispatch.updateGames(roomId, gameList);
  }

  addMessage(roomId, message) {
    NormalizeService.normalizeUserMessage(message);

    Dispatch.addMessage(roomId, message);
  }

  userJoined(roomId, user) {
    Dispatch.userJoined(roomId, user);
  }

  userLeft(roomId, name) {
    Dispatch.userLeft(roomId, name);
  }
}
