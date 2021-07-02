import { webClient } from "websocket";

export default class RoomsService {
  static joinRoom(roomId) {
    webClient.commands.session.joinRoom(roomId);
  }

  static leaveRoom(roomId) {
    webClient.commands.room.leaveRoom(roomId);
  }

  static roomSay(roomId, message) {
    webClient.commands.room.roomSay(roomId, message);
  }
}