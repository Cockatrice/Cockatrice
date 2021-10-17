import { ServerConnectParams } from "store";
import { StatusEnum } from "types";

import { RoomPersistence, SessionPersistence } from '../persistence';
import webClient from "../WebClient";
import { guid } from "../utils";

export class SessionCommands {
  static connect(options: ServerConnectParams) {
    webClient.socket.updateStatus(StatusEnum.CONNECTING, "Connecting...");
    webClient.connect(options);
  }

  static disconnect() {
    webClient.socket.updateStatus(StatusEnum.DISCONNECTING, "Disconnecting...");
    webClient.socket.disconnect();
  }

  static login() {
    const loginConfig = {
      ...webClient.clientConfig,
      "userName" : webClient.options.user,
      "password" : webClient.options.pass,
      "clientid" : guid()
    };

    const CmdLogin = webClient.protobuf.controller.Command_Login.create(loginConfig);

    const command = webClient.protobuf.controller.SessionCommand.create({
      ".Command_Login.ext" : CmdLogin
    });

    webClient.protobuf.sendSessionCommand(command, raw => {
      const resp = raw[".Response_Login.ext"];

      switch(raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { buddyList, ignoreList, userInfo } = resp;

          SessionPersistence.updateBuddyList(buddyList);
          SessionPersistence.updateIgnoreList(ignoreList);
          SessionPersistence.updateUser(userInfo);

          SessionCommands.listUsers();
          SessionCommands.listRooms();

          webClient.socket.updateStatus(StatusEnum.LOGGEDIN, "Logged in.");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespClientUpdateRequired:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: missing features");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword:
        case webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: incorrect username or password");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespWouldOverwriteOldSession:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: duplicated user session");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespUserIsBanned:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: banned user");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: registration required");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespClientIdRequired:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: missing client ID");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespContextError:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: server error");
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespAccountNotActivated:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: account not activated");
          break;

        default:
          webClient.socket.updateStatus(StatusEnum.DISCONNECTED, "Login failed: unknown error " + raw.responseCode);
      }
    });
  }

  static listUsers() {
    const CmdListUsers = webClient.protobuf.controller.Command_ListUsers.create();

    const sc = webClient.protobuf.controller.SessionCommand.create({
      ".Command_ListUsers.ext" : CmdListUsers
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      const { responseCode } = raw;
      const response = raw[".Response_ListUsers.ext"];

      if (response) {
        switch (responseCode) {
          case webClient.protobuf.controller.Response.ResponseCode.RespOk:
            SessionPersistence.updateUsers(response.userList);
            break;
          default:
            console.log(`Failed to fetch Server Rooms [${responseCode}] : `, raw);
        }
      }

    });
  }

  static listRooms() {
    const CmdListRooms = webClient.protobuf.controller.Command_ListRooms.create();

    const sc = webClient.protobuf.controller.SessionCommand.create({
      ".Command_ListRooms.ext" : CmdListRooms
    });

    webClient.protobuf.sendSessionCommand(sc);
  }

  static joinRoom(roomId: string) {
    const CmdJoinRoom = webClient.protobuf.controller.Command_JoinRoom.create({
      "roomId" : roomId
    });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      ".Command_JoinRoom.ext" : CmdJoinRoom
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch(responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { roomInfo } = raw[".Response_JoinRoom.ext"];

          RoomPersistence.joinRoom(roomInfo);
          return;
        case webClient.protobuf.controller.Response.ResponseCode.RespNameNotFound:
          error = "Failed to join the room: it doesn\"t exist on the server.";
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespContextError:
          error = "The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.";
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserLevelTooLow:
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

  static addToBuddyList(userName) {
    this.addToList('buddy', userName);
  }

  static removeFromBuddyList(userName) {
    this.removeFromList('buddy', userName);
  }

  static addToIgnoreList(userName) {
    this.addToList('ignore', userName);
  }

  static removeFromIgnoreList(userName) {
    this.removeFromList('ignore', userName);
  }

  static addToList(list: string, userName: string) {
    const CmdAddToList = webClient.protobuf.controller.Command_AddToList.create({ list, userName });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      ".Command_AddToList.ext" : CmdAddToList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static removeFromList(list: string, userName: string) {
    const CmdRemoveFromList = webClient.protobuf.controller.Command_RemoveFromList.create({ list, userName });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      ".Command_RemoveFromList.ext" : CmdRemoveFromList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static viewLogHistory(filters) {
    const CmdViewLogHistory = webClient.protobuf.controller.Command_ViewLogHistory.create(filters);

    const sc = webClient.protobuf.controller.ModeratorCommand.create({
      ".Command_ViewLogHistory.ext" : CmdViewLogHistory
    });

    webClient.protobuf.sendModeratorCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch(responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { logMessage } = raw[".Response_ViewLogHistory.ext"];

          console.log("Response_ViewLogHistory: ", logMessage)
          SessionPersistence.viewLogs(logMessage)
          return;
        default:
          error = "Failed to retrieve log history.";
          break;
      }
      
      if (error) {
        console.error(responseCode, error);
      }
    });
  }
}
