import type { IRoomsRequest } from '@app/websocket';
import { RoomCommands, SessionCommands } from '@app/websocket';

export class RoomsRequestImpl implements IRoomsRequest {
  joinRoom(roomId: number): void {
    SessionCommands.joinRoom(roomId);
  }

  leaveRoom(roomId: number): void {
    RoomCommands.leaveRoom(roomId);
  }

  roomSay(roomId: number, message: string): void {
    RoomCommands.roomSay(roomId, message);
  }
}
