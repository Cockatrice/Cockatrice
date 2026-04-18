import { RoomCommands, SessionCommands } from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';

export class RoomsRequestImpl implements WebsocketTypes.IRoomsRequest {
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
