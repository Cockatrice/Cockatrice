import { RoomCommands, SessionCommands } from '@app/websocket';
import { WebsocketTypes } from '@app/websocket/types';
import type { App } from '@app/types';

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

  createGame(roomId: number, params: App.CreateGameParams): void {
    RoomCommands.createGame(roomId, params);
  }

  joinGame(roomId: number, params: App.JoinGameParams): void {
    RoomCommands.joinGame(roomId, params);
  }
}
