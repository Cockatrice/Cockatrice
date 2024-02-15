import { Command_LeaveRoom, Command_RoomSay, RoomCommand, Response } from 'protoFiles';
import { RoomPersistence } from '../persistence';
import webClient from '../WebClient';

export class RoomCommands {
  static roomSay(roomId: number, message: string): void {
    const trimmed = message.trim();

    if (!trimmed) {
      return;
    }

    const CmdRoomSay = Command_RoomSay.create({
      'message': trimmed
    });

    const rc = RoomCommand.create({
      '.Command_RoomSay.ext': CmdRoomSay
    });

    webClient.protobuf.sendRoomCommand(roomId, rc);
  }

  static leaveRoom(roomId: number): void {
    const CmdLeaveRoom = Command_LeaveRoom.create();

    const rc = RoomCommand.create({
      '.Command_LeaveRoom.ext': CmdLeaveRoom
    });

    webClient.protobuf.sendRoomCommand(roomId, rc, (raw) => {
      const { responseCode } = raw;

      switch (responseCode) {
        case Response.ResponseCode.RespOk:
          RoomPersistence.leaveRoom(roomId);
          break;
        default:
          console.log(`Failed to leave Room ${roomId} [${responseCode}] : `, raw);
      }
    });
  }
}
