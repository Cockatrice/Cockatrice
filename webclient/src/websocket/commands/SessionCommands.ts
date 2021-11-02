import {StatusEnum, WebSocketConnectReason, WebSocketOptions} from 'types';

import { RoomPersistence, SessionPersistence } from '../persistence';
import webClient from '../WebClient';
import { guid, hashPassword } from '../utils';
import {
  AccountActivationParams,
  ForgotPasswordChallengeParams,
  ForgotPasswordParams,
  ForgotPasswordResetParams,
  RequestPasswordSaltParams,
  ServerRegisterParams
} from '../../store';
import NormalizeService from '../utils/NormalizeService';

export class SessionCommands {
  static connect(options: WebSocketOptions, reason: WebSocketConnectReason): void {
    switch (reason) {
      case WebSocketConnectReason.LOGIN:
      case WebSocketConnectReason.REGISTER:
      case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      case WebSocketConnectReason.PASSWORD_RESET:
        SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
        break;
      default:
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Unknown Connection Attempt: ' + reason);
        return;
    }

    webClient.connect({ ...options, reason });
  }

  static disconnect(): void {
    webClient.disconnect();
  }

  static login(passwordSalt?: string): void {
    const loginConfig: any = {
      ...webClient.clientConfig,
      userName: webClient.options.user,
      clientid: guid()
    };

    if (passwordSalt) {
      loginConfig.hashedPassword = hashPassword(passwordSalt, webClient.options.pass);
    } else {
      loginConfig.password = webClient.options.pass;
    }

    const CmdLogin = webClient.protobuf.controller.Command_Login.create(loginConfig);

    const command = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Login.ext': CmdLogin
    });

    webClient.protobuf.sendSessionCommand(command, raw => {
      const resp = raw['.Response_Login.ext'];

      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        const { buddyList, ignoreList, userInfo } = resp;

        SessionPersistence.updateBuddyList(buddyList);
        SessionPersistence.updateIgnoreList(ignoreList);
        SessionPersistence.updateUser(userInfo);

        SessionCommands.listUsers();
        SessionCommands.listRooms();

        SessionCommands.updateStatus(StatusEnum.LOGGED_IN, 'Logged in.');
        return;
      }

      switch (raw.responseCode) {
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

      SessionCommands.disconnect();
    });
  }

  static requestPasswordSalt(): void {
    const options = webClient.options as unknown as RequestPasswordSaltParams;

    const registerConfig = {
      ...webClient.clientConfig,
      userName: options.user,
    };

    const CmdRequestPasswordSalt = webClient.protobuf.controller.Command_RequestPasswordSalt.create(registerConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_RequestPasswordSalt.ext': CmdRequestPasswordSalt
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      switch (raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk:
          const passwordSalt = raw['.Response_PasswordSalt.ext'].passwordSalt;
          SessionCommands.login(passwordSalt);
          break;

        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: incorrect username or password');
          SessionCommands.disconnect();
          break;

        default:
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
          SessionCommands.disconnect();
          break;
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
      clientid: 'webatrice'
    };

    const CmdRegister = webClient.protobuf.controller.Command_Register.create(registerConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Register.ext': CmdRegister
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAccepted) {
        SessionCommands.login();
        return;
      }

      let error;

      switch (raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAcceptedNeedsActivation:
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
          console.error('ResponseCode.RespUsernameInvalid', raw.reasonStr);
          error = 'Invalid username';
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationFailed:
        default:
          console.error('ResponseCode Type', raw.responseCode);
          error = 'Registration failed due to a server issue';
          break;
      }

      if (error) {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, `Registration Failed: ${error}`);
      }

      SessionCommands.disconnect();
    });
  };

  static activateAccount(): void {
    const options = webClient.options as unknown as AccountActivationParams;

    const accountActivationConfig = {
      ...webClient.clientConfig,
      userName: options.user,
      clientid: options.clientid,
      token: options.activationCode
    };

    const CmdActivate = webClient.protobuf.controller.Command_Activate.create(accountActivationConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Activate.ext': CmdActivate
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespActivationAccepted) {
        SessionCommands.login();
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Account Activation Failed');
        SessionCommands.disconnect();
        SessionPersistence.accountActivationFailed();
      }
    });
  }

  static resetPasswordRequest(): void {
    const options = webClient.options as unknown as ForgotPasswordParams;

    const forgotPasswordConfig = {
      ...webClient.clientConfig,
      userName: options.user,
      clientid: options.clientid
    };

    const CmdForgotPasswordRequest = webClient.protobuf.controller.Command_ForgotPasswordRequest.create(forgotPasswordConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordRequest.ext': CmdForgotPasswordRequest
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        const resp = raw['.Response_ForgotPasswordRequest.ext'];

        if (resp.challengeEmail) {
          SessionPersistence.resetPasswordChallenge();
        } else {
          SessionPersistence.resetPassword();
        }
      } else {
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static resetPasswordChallenge(): void {
    const options = webClient.options as unknown as ForgotPasswordChallengeParams;

    const forgotPasswordChallengeConfig = {
      ...webClient.clientConfig,
      userName: options.user,
      clientid: options.clientid,
      email: options.email
    };

    const CmdForgotPasswordChallenge = webClient.protobuf.controller.Command_ForgotPasswordChallenge.create(forgotPasswordChallengeConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordChallenge.ext': CmdForgotPasswordChallenge
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        SessionPersistence.resetPassword();
      } else {
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static resetPassword(): void {
    const options = webClient.options as unknown as ForgotPasswordResetParams;

    const forgotPasswordResetConfig = {
      ...webClient.clientConfig,
      userName: options.user,
      clientid: options.clientid,
      token: options.token,
      newPassword: options.newPassword
    };

    const CmdForgotPasswordReset = webClient.protobuf.controller.Command_ForgotPasswordReset.create(forgotPasswordResetConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordReset.ext': CmdForgotPasswordReset
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        SessionPersistence.resetPasswordSuccess();
      } else {
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static listUsers(): void {
    const CmdListUsers = webClient.protobuf.controller.Command_ListUsers.create();

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ListUsers.ext': CmdListUsers
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
      '.Command_ListRooms.ext': CmdListRooms
    });

    webClient.protobuf.sendSessionCommand(sc);
  }

  static joinRoom(roomId: number): void {
    const CmdJoinRoom = webClient.protobuf.controller.Command_JoinRoom.create({ roomId });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_JoinRoom.ext': CmdJoinRoom
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch (responseCode) {
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
      '.Command_AddToList.ext': CmdAddToList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static removeFromList(list: string, userName: string): void {
    const CmdRemoveFromList = webClient.protobuf.controller.Command_RemoveFromList.create({ list, userName });

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_RemoveFromList.ext': CmdRemoveFromList
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static viewLogHistory(filters): void {
    const CmdViewLogHistory = webClient.protobuf.controller.Command_ViewLogHistory.create(filters);

    const sc = webClient.protobuf.controller.ModeratorCommand.create({
      '.Command_ViewLogHistory.ext': CmdViewLogHistory
    });

    webClient.protobuf.sendModeratorCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch (responseCode) {
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
