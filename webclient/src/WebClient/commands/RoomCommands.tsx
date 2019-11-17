import { WebClient } from '../WebClient'; 

export class RoomCommands {
  private webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  roomSay(roomId, message) {
    var CmdRoomSay = this.webClient.pb.Command_RoomSay.create({
      "message" : message
    });

    var rc = this.webClient.pb.RoomCommand.create({
      ".Command_RoomSay.ext" : CmdRoomSay
    });

    this.webClient.sendRoomCommand(roomId, rc);
  }
}