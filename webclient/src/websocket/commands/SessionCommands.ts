import {StatusEnum} from 'types';

import {RoomPersistence, SessionPersistence} from '../persistence';
import webClient from '../WebClient';
import {guid} from '../utils';
import {WebSocketConnectReason, WebSocketOptions} from "../services/WebSocketService";
import {ServerRegisterParams} from "../../store";
import NormalizeService from "../utils/NormalizeService";

export class SessionCommands {
  static connect(options: WebSocketOptions, reason: WebSocketConnectReason): void {
    switch (reason) {
      case WebSocketConnectReason.CONNECT:
        SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
        break;
      case WebSocketConnectReason.REGISTER:
        SessionCommands.updateStatus(StatusEnum.REGISTERING, 'Registering...');
        break;
      case WebSocketConnectReason.RECOVER_PASSWORD:
        SessionCommands.updateStatus(StatusEnum.RECOVERING_PASSWORD, 'Recovering Password...');
        break;
      default:
        console.error('Connection Failed', reason);
        break;
    }

    webClient.connect({ ...options, reason });
  }

  static disconnect(): void {
    SessionCommands.updateStatus(StatusEnum.DISCONNECTING, 'Disconnecting...');
    webClient.disconnect();
  }

  static login(): void {
    const loginConfig = {
      ...webClient.clientConfig,
      userName: webClient.options.user,
      password: webClient.options.pass,
      clientid: guid()
    };

    const CmdLogin = webClient.protobuf.controller.Command_Login.create(loginConfig);

    const command = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Login.ext' : CmdLogin
    });

    webClient.protobuf.sendSessionCommand(command, raw => {
      const resp = raw['.Response_Login.ext'];

      switch(raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { buddyList, ignoreList, userInfo } = resp;

          SessionPersistence.updateBuddyList(buddyList);
          SessionPersistence.updateIgnoreList(ignoreList);
          SessionPersistence.updateUser(userInfo);

          SessionCommands.listUsers();
          SessionCommands.listRooms();

          SessionCommands.updateStatus(StatusEnum.LOGGEDIN, 'Logged in.');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespClientUpdateRequired:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: missing features');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespWrongPassword:
        case webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: incorrect username or password');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespWouldOverwriteOldSession:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: duplicated user session');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespUserIsBanned:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: banned user');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: registration required');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespClientIdRequired:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: missing client ID');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespContextError:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: server error');
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespAccountNotActivated:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: account not activated');
          SessionPersistence.accountAwaitingActivation();
          break;

        default:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Login failed: unknown error: ${raw.responseCode}`);
      }
    });
  }

  static register(): void {
    const options = webClient.options as unknown as ServerRegisterParams;

    const registerConfig = {
      ...webClient.clientConfig,
      userName: options.user,
      password: options.pass,
      email: options.email,
      country: options.country,
      realName: options.realName,
      clientid: guid()
    };

    const CmdRegister = webClient.protobuf.controller.Command_Register.create(registerConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Register.ext' : CmdRegister
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      let error;

      switch (raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAccepted:
          SessionCommands.login();
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAcceptedNeedsActivation:
          SessionCommands.updateStatus(StatusEnum.REGISTERED, "Registration Successful");
          SessionPersistence.accountAwaitingActivation();
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationDisabled:
          error = 'Registration is currently disabled';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserAlreadyExists:
          error = 'There is already an existing user with this username';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespEmailRequiredToRegister:
          error = 'A valid email address is required to register';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespEmailBlackListed:
          error = 'The email address provider used has been blocked from use';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespTooManyRequests:
          error = 'This email address already has the maximum number of accounts you can register';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespPasswordTooShort:
          error = 'Your password was too short';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserIsBanned:
          error = NormalizeService.normalizeBannedUserError(raw.reasonStr, raw.endTime);
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid:
          console.error("ResponseCode.RespUsernameInvalid", raw.reasonStr);
          error = 'Invalid username';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationFailed:
        default:
          console.error("ResponseCode Type", raw.responseCode);
          error = 'Registration failed due to a server issue';
          break;
      }

      if (error) {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Registration Failed: ${error}`);
      }
    });
  };

  static listUsers(): void {
    const CmdListUsers = webClient.protobuf.controller.Command_ListUsers.create();

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ListUsers.ext' : CmdListUsers
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      const { responseCode } = raw;
      const response = raw['.Response_ListUsers.ext'];

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

  static listRooms(): void {
    const CmdListRooms = webClient.protobuf.controller.Command_ListRooms.create();

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ListRooms.ext' : CmdListRooms
    });

    webClient.protobuf.sendSessionCommand(sc);
  }

  static joinRoom(roomId: number): void {
    const CmdJoinRoom = webClient.protobuf.controller.Command_JoinRoom.create({ roomId });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_JoinRoom.ext' : CmdJoinRoom
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch(responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { roomInfo } = raw['.Response_JoinRoom.ext'];

          RoomPersistence.joinRoom(roomInfo);
          return;
        case webClient.protobuf.controller.Response.ResponseCode.RespNameNotFound:
          error = 'Failed to join the room: it doesn\'t exist on the server.';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespContextError:
          error = 'The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserLevelTooLow:
          error = 'You do not have the required permission to join this room.';
          break;
        default:
          error = 'Failed to join the room due to an unknown error.';
          break;
      }
      
      if (error) {
        console.error(responseCode, error);
      }
    });
  }

  static addToBuddyList(userName: string): void {
    this.addToList('buddy', userName);
  }

  static removeFromBuddyList(userName: string): void {
    this.removeFromList('buddy', userName);
  }

  static addToIgnoreList(userName: string): void {
    this.addToList('ignore', userName);
  }

  static removeFromIgnoreList(userName: string): void {
    this.removeFromList('ignore', userName);
  }

  static addToList(list: string, userName: string): void {
    const CmdAddToList = webClient.protobuf.controller.Command_AddToList.create({ list, userName });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_AddToList.ext' : CmdAddToList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static removeFromList(list: string, userName: string): void {
    const CmdRemoveFromList = webClient.protobuf.controller.Command_RemoveFromList.create({ list, userName });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_RemoveFromList.ext' : CmdRemoveFromList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static viewLogHistory(filters): void {
    const CmdViewLogHistory = webClient.protobuf.controller.Command_ViewLogHistory.create(filters);

    const sc = webClient.protobuf.controller.ModeratorCommand.create({
      '.Command_ViewLogHistory.ext' : CmdViewLogHistory
    });

    webClient.protobuf.sendModeratorCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch(responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const { logMessage } = raw['.Response_ViewLogHistory.ext'];
          SessionPersistence.viewLogs(logMessage)
          return;
        default:
          error = 'Failed to retrieve log history.';
          break;
      }
      
      if (error) {
        console.error(responseCode, error);
      }
    });
  }

  static updateStatus(status: StatusEnum, description: string): void {
    webClient.updateStatus(status, description);
  }
}
