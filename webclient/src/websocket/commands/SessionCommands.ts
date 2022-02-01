import { HostDTO } from 'services';
import { StatusEnum, WebSocketConnectReason, WebSocketConnectOptions } from 'types';

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
  static connect(options: WebSocketConnectOptions, reason: WebSocketConnectReason): void {
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
    const { userName, password, hashedPassword } = webClient.options;

    const loginConfig: any = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
    };

    if (passwordSalt) {
      loginConfig.hashedPassword = hashedPassword || hashPassword(passwordSalt, password);
    } else {
      loginConfig.password = password;
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
        SessionPersistence.loginSuccessful(loginConfig);

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

      SessionPersistence.loginFailed();
      SessionCommands.disconnect();
    });
  }

  static requestPasswordSalt(): void {
    const { userName } = webClient.options as unknown as RequestPasswordSaltParams;

    const registerConfig = {
      ...webClient.clientConfig,
      userName,
    };

    const CmdRequestPasswordSalt = webClient.protobuf.controller.Command_RequestPasswordSalt.create(registerConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_RequestPasswordSalt.ext': CmdRequestPasswordSalt
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      switch (raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespOk: {
          const passwordSalt = raw['.Response_PasswordSalt.ext']?.passwordSalt;

          switch (webClient.options.reason) {
            case WebSocketConnectReason.REGISTER: {
              SessionCommands.register(passwordSalt);
              break;
            }

            case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
              SessionCommands.activateAccount(passwordSalt);
              break;
            }

            case WebSocketConnectReason.PASSWORD_RESET: {
              SessionCommands.resetPassword(passwordSalt);
              break;
            }

            case WebSocketConnectReason.LOGIN:
            default: {
              SessionCommands.login(passwordSalt);
            }
          }

          return;
        }
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationRequired: {
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: registration required');
          break;
        }
        default: {
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
        }
      }

      switch (webClient.options.reason) {
        case WebSocketConnectReason.REGISTER: {
          SessionPersistence.registrationFailed('Failed to retrieve password salt');
          break;
        }

        case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
          SessionPersistence.accountActivationFailed();
          break;
        }

        case WebSocketConnectReason.PASSWORD_RESET: {
          SessionPersistence.resetPasswordFailed();
          break;
        }

        case WebSocketConnectReason.LOGIN:
        default: {
          SessionPersistence.loginFailed();
        }
      }

      SessionCommands.disconnect();
    });
  }

  static register(passwordSalt?: string): void {
    const { userName, password, email, country, realName } = webClient.options as unknown as ServerRegisterParams;

    const registerConfig: any = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
      email,
      country,
      realName,
    };

    if (passwordSalt) {
      registerConfig.hashedPassword = hashPassword(passwordSalt, password);
    } else {
      registerConfig.password = password;
    }

    const CmdRegister = webClient.protobuf.controller.Command_Register.create(registerConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Register.ext': CmdRegister
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAccepted) {
        SessionCommands.login(passwordSalt);
        return;
      }

      switch (raw.responseCode) {
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAcceptedNeedsActivation:
          SessionPersistence.accountAwaitingActivation();
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserAlreadyExists:
          SessionPersistence.registrationUserNameError('Username is taken');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid:
          console.error('ResponseCode.RespUsernameInvalid', raw.reasonStr);
          SessionPersistence.registrationUserNameError('Invalid username');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespPasswordTooShort:
          SessionPersistence.registrationPasswordError('Your password was too short');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespEmailRequiredToRegister:
          SessionPersistence.registrationRequiresEmail();
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespEmailBlackListed:
          SessionPersistence.registrationEmailError('This email provider has been blocked');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespTooManyRequests:
          SessionPersistence.registrationEmailError('Max accounts reached for this email');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationDisabled:
          SessionPersistence.registrationFailed('Registration is currently disabled');
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespUserIsBanned:
          SessionPersistence.registrationFailed(NormalizeService.normalizeBannedUserError(raw.reasonStr, raw.endTime));
          break;
        case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationFailed:
        default:
          SessionPersistence.registrationFailed('Registration failed due to a server issue');
          break;
      }

      SessionCommands.disconnect();
    });
  };

  static activateAccount(passwordSalt?: string): void {
    const { userName, token } = webClient.options as unknown as AccountActivationParams;

    const accountActivationConfig = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
      token,
    };

    const CmdActivate = webClient.protobuf.controller.Command_Activate.create(accountActivationConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_Activate.ext': CmdActivate
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespActivationAccepted) {
        SessionPersistence.accountActivationSuccess();
        SessionCommands.login(passwordSalt);
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, 'Account Activation Failed');
        SessionCommands.disconnect();
        SessionPersistence.accountActivationFailed();
      }
    });
  }

  static resetPasswordRequest(): void {
    const { userName } = webClient.options as unknown as ForgotPasswordParams;

    const forgotPasswordConfig = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
    };

    const CmdForgotPasswordRequest = webClient.protobuf.controller.Command_ForgotPasswordRequest.create(forgotPasswordConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordRequest.ext': CmdForgotPasswordRequest
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        const resp = raw['.Response_ForgotPasswordRequest.ext'];

        if (resp.challengeEmail) {
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
          SessionPersistence.resetPasswordChallenge();
        } else {
          SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
          SessionPersistence.resetPassword();
        }
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static resetPasswordChallenge(): void {
    const { userName, email } = webClient.options as unknown as ForgotPasswordChallengeParams;

    const forgotPasswordChallengeConfig = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
      email,
    };

    const CmdForgotPasswordChallenge = webClient.protobuf.controller.Command_ForgotPasswordChallenge.create(forgotPasswordChallengeConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordChallenge.ext': CmdForgotPasswordChallenge
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPassword();
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static resetPassword(passwordSalt?: string): void {
    const { userName, token, newPassword } = webClient.options as unknown as ForgotPasswordResetParams;

    const forgotPasswordResetConfig: any = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
      token,
    };

    if (passwordSalt) {
      forgotPasswordResetConfig.hashedNewPassword = hashPassword(passwordSalt, newPassword);
    } else {
      forgotPasswordResetConfig.newPassword = newPassword;
    }

    const CmdForgotPasswordReset = webClient.protobuf.controller.Command_ForgotPasswordReset.create(forgotPasswordResetConfig);

    const sc = webClient.protobuf.controller.SessionCommand.create({
      '.Command_ForgotPasswordReset.ext': CmdForgotPasswordReset
    });

    webClient.protobuf.sendSessionCommand(sc, raw => {
      if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespOk) {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordSuccess();
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
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
