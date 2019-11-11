import { Dispatch } from 'store/rooms';

import { WebClient } from '../WebClient';

export class RoomService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  joinRoom(roomInfo) {
    Dispatch.joinRoom(roomInfo);
  }

  updateRooms(rooms) {
    Dispatch.updateRooms(rooms);
  }

  updateGames(roomId, gameList) {
    Dispatch.updateGames(roomId, gameList);
  }

  addMessage(roomId, message) {
    const { name } = message;

    if (name) {
      message.message = `${name}: ${message.message}`;
    }

    Dispatch.addMessage(roomId, message);
  }
}
