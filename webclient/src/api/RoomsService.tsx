import { RoomCommands, SessionCommands } from "websocket";

export default class RoomsService {
  static joinRoom(roomId) {
    SessionCommands.joinRoom(roomId);
  }

  static leaveRoom(roomId) {
    RoomCommands.leaveRoom(roomId);
  }

  static roomSay(roomId, message) {
    RoomCommands.roomSay(roomId, message);
  }
}
