import { RoomCommands, SessionCommands } from 'websocket';

export class RoomsService {
  static joinRoom(roomId: number): void {
    SessionCommands.joinRoom(roomId);
  }

  static leaveRoom(roomId: number): void {
    RoomCommands.leaveRoom(roomId);
  }

  static roomSay(roomId: number, message: string): void {
    RoomCommands.roomSay(roomId, message);
  }
}
