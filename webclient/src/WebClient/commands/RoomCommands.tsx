import * as _ from 'lodash';

import { WebClient } from "../WebClient"; 

export class RoomCommands {
  private webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  roomSay(roomId, message) {
    const trimmed = _.trim(message);
    
    if (!trimmed) return;

    var CmdRoomSay = this.webClient.pb.Command_RoomSay.create({
      "message" : trimmed
    });

    var rc = this.webClient.pb.RoomCommand.create({
      ".Command_RoomSay.ext" : CmdRoomSay
    });

    this.webClient.sendRoomCommand(roomId, rc);
  }
}