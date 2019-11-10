import { Dispatch } from 'store/rooms';

import { WebClient } from '../WebClient';

export class RoomService {
  webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  updateRooms(rooms) {
    Dispatch.updateRooms(rooms);
  }

  joinRoom(roomId) {
    Dispatch.joinRoom(roomId);
  }

  addMessage(roomId, message) {
    const { name } = message;

    if (name) {
      message.message = `${name}: ${message.message}`;
    }

    Dispatch.addMessage(roomId, message);
  }
}
