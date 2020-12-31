import webClient from "WebClient/WebClient";

export class RoomsService {
  static joinRoom(roomId) {
    webClient.commands.session.joinRoom(roomId);
  }

  static roomSay(roomId, message) {
    webClient.commands.room.roomSay(roomId, message);
  }
}