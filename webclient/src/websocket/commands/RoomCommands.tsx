import * as _ from 'lodash';

import { RoomPersistence } from '../persistence';
import webClient from "../WebClient";

export class RoomCommands {
  static roomSay(roomId, message) {
    const trimmed = _.trim(message);
    
    if (!trimmed) return;

    var CmdRoomSay = webClient.protobuf.controller.Command_RoomSay.create({
      "message" : trimmed
    });

    var rc = webClient.protobuf.controller.RoomCommand.create({
      ".Command_RoomSay.ext" : CmdRoomSay
    });

    webClient.protobuf.sendRoomCommand(roomId, rc);
  }

  static leaveRoom(roomId) {
    var CmdLeaveRoom = webClient.protobuf.controller.Command_LeaveRoom.create();

    var rc = webClient.protobuf.controller.RoomCommand.create({
      ".Command_LeaveRoom.ext" : CmdLeaveRoom
    });

    webClient.protobuf.sendRoomCommand(roomId, rc, (raw) => {
      const { responseCode } = raw;

      switch (responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          RoomPersistence.leaveRoom(roomId);
          break;
        default:
          console.log(`Failed to leave Room ${roomId} [${responseCode}] : `, raw);
      }
    });
  }
}
