import { StatusEnum } from 'types';

import { WebClient } from '../WebClient'; 
import { guid } from '../util';

const defaultLoginConfig = {
  "clientver" : "webclient-1.0 (2019-10-31)",
  "clientfeatures" : [
    "client_id",
    "client_ver",
    "feature_set",
    "room_chat_history",
    "client_warnings",
    /* unimplemented features */
    "forgot_password",
    "idle_client",
    "mod_log_lookup",
    "user_ban_history",
    // satisfy server reqs for POC
    "websocket",
    "2.6.1_min_version",
    "2.7.0_min_version",
  ]
}

export class SessionCommands {
  private webClient: WebClient;

  constructor(webClient) {
    this.webClient = webClient;
  }

  login() {
    const loginConfig = {
      ...this.webClient.clientConfig,
      "userName" : this.webClient.options.user,
      "password" : this.webClient.options.pass,
      "clientid" : guid()
    };

    const CmdLogin = this.webClient.pb.Command_Login.create(loginConfig);

    const command = this.webClient.pb.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    this.webClient.sendSessionCommand(command, raw => {
      const resp = raw[".Response_Login.ext"];

      this.webClient.debug(() =>  console.log('.Response_Login.ext', resp));

      switch(raw.responseCode) {
        case this.webClient.pb.Response.ResponseCode.RespOk:
          const { userInfo } = resp;
          this.webClient.services.session.updateUser(userInfo);
          this.webClient.commands.session.listUsers();
          this.webClient.commands.session.listRooms();
          this.webClient.updateStatus(StatusEnum.LOGGEDIN, 'Logged in.');
          this.webClient.startPingLoop();
          break;

        case this.webClient.pb.Response.ResponseCode.RespClientUpdateRequired:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing features');
          break;

        case this.webClient.pb.Response.ResponseCode.RespWrongPassword:
        case this.webClient.pb.Response.ResponseCode.RespUsernameInvalid:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: incorrect username or password');
          break;

        case this.webClient.pb.Response.ResponseCode.RespWouldOverwriteOldSession:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: duplicated user session');
          break;

        case this.webClient.pb.Response.ResponseCode.RespUserIsBanned:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: banned user');
          break;

        case this.webClient.pb.Response.ResponseCode.RespRegistrationRequired:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: registration required');
          break;

        case this.webClient.pb.Response.ResponseCode.RespClientIdRequired:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: missing client ID');
          break;

        case this.webClient.pb.Response.ResponseCode.RespContextError:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: server error');
          break;

        case this.webClient.pb.Response.ResponseCode.RespAccountNotActivated:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: account not activated');
          break;

        default:
          this.webClient.updateStatus(StatusEnum.DISCONNECTING, 'Login failed: unknown error ' + raw.responseCode);
      }
    });
  }

  listUsers() {
    const CmdListUsers = this.webClient.pb.Command_ListUsers.create();

    const sc = this.webClient.pb.SessionCommand.create({
      ".Command_ListUsers.ext" : CmdListUsers
    });

    this.webClient.sendSessionCommand(sc, raw => {
      const { responseCode } = raw;
      const response = raw['.Response_ListUsers.ext'];

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

          this.webClient.services.room.joinRoom(roomInfo);
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