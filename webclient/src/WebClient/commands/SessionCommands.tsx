import { WebClient } from '../WebClient'; 

export class SessionCommands {
  private webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  listUsers() {
    const CmdListUsers = this.webClient.pb.Command_ListUsers.create();

    const sc = this.webClient.pb.SessionCommand.create({
      ".Command_ListUsers.ext" : CmdListUsers
    });

    this.webClient.sendSessionCommand(sc, raw => {
      const { responseCode } = raw;
      const response = raw['.Response_ListUsers.ext'];

      console.log('ListUsers', raw);

      if (response) {
        switch (responseCode) {
          case this.webClient.pb.Response.ResponseCode.RespOk:
            this.webClient.services.session.updateUsers(response.userList);
            break;
          default:
            console.log(`Failed to fetch Server Rooms [${responseCode}] : `, raw);
        }
      }

    });
  }

  listRooms() {
    const CmdListRooms = this.webClient.pb.Command_ListRooms.create();

    const sc = this.webClient.pb.SessionCommand.create({
      ".Command_ListRooms.ext" : CmdListRooms
    });

    this.webClient.sendSessionCommand(sc);
  }

  joinRoom(roomId: string) {
    const CmdJoinRoom = this.webClient.pb.Command_JoinRoom.create({
      "roomId" : roomId
    });

    const sc = this.webClient.pb.SessionCommand.create({
      ".Command_JoinRoom.ext" : CmdJoinRoom
    });

    this.webClient.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch(responseCode) {
        case this.webClient.pb.Response.ResponseCode.RespOk:
          const { roomInfo } = raw['.Response_JoinRoom.ext'];

          this.webClient.services.rooms.joinRoom(roomInfo);
          this.webClient.debug(() => console.log('Join Room: ', roomInfo.name));
          return;
        case this.webClient.pb.Response.ResponseCode.RespNameNotFound:
          error = "Failed to join the room: it doesn't exist on the server.";
          break;
        case this.webClient.pb.Response.ResponseCode.RespContextError:
          error = "The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.";
          break;
        case this.webClient.pb.Response.ResponseCode.RespUserLevelTooLow:
          error = "You do not have the required permission to join this room.";
          break;
        default:
          error = "Failed to join the room due to an unknown error.";
          break;
      }
      
      if (error) {
        console.error(responseCode, error);
      }
    });
  }
}