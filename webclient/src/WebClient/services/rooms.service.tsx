import { Dispatch } from 'store/rooms';

import { WebClient } from '../WebClient';

export class RoomsService {
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
}
