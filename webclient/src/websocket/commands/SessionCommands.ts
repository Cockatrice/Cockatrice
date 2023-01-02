import { HostDTO } from 'services';
import {
  StatusEnum,
  WebSocketConnectReason,
  WebSocketConnectOptions,
} from 'types';

import { RoomPersistence, SessionPersistence } from '../persistence';
import webClient from '../WebClient';
import { guid, hashPassword } from '../utils';
import {
  AccountActivationParams,
  ForgotPasswordChallengeParams,
  ForgotPasswordParams,
  ForgotPasswordResetParams,
  RequestPasswordSaltParams,
  ServerRegisterParams,
} from '../../store';
import NormalizeService from '../utils/NormalizeService';
import {
  Response,
  Command_Login,
  SessionCommand,
  Command_RequestPasswordSalt,
  Command_Register,
  Command_Activate,
  Command_ForgotPasswordRequest,
  Command_ForgotPasswordChallenge,
  Command_ForgotPasswordReset,
  Command_ListUsers,
  Command_ListRooms,
  Command_JoinRoom,
  Command_AddToList,
  Command_RemoveFromList,
  Command_ViewLogHistory,
  ModeratorCommand,
} from 'protoFiles';

export class SessionCommands {
  static connect(
    options: WebSocketConnectOptions,
    reason: WebSocketConnectReason
  ): void {
    switch (reason) {
      case WebSocketConnectReason.LOGIN:
      case WebSocketConnectReason.REGISTER:
      case WebSocketConnectReason.ACTIVATE_ACCOUNT:
      case WebSocketConnectReason.PASSWORD_RESET_REQUEST:
      case WebSocketConnectReason.PASSWORD_RESET_CHALLENGE:
      case WebSocketConnectReason.PASSWORD_RESET:
        SessionCommands.updateStatus(StatusEnum.CONNECTING, 'Connecting...');
        break;
      case WebSocketConnectReason.TEST_CONNECTION:
        webClient.testConnect({ ...options });
        return;
      default:
        SessionCommands.updateStatus(
          StatusEnum.DISCONNECTED,
          'Unknown Connection Attempt: ' + reason
        );
        return;
    }

    webClient.connect({ ...options, reason });
  }

  static disconnect(): void {
    webClient.disconnect();
  }

  static login(options: WebSocketConnectOptions, passwordSalt?: string): void {
    const { userName, password, hashedPassword } = options;

    const loginConfig: any = {
      ...webClient.clientConfig,
      clientid: 'webatrice',
      userName,
    };

    if (passwordSalt) {
      loginConfig.hashedPassword =
        hashedPassword || hashPassword(passwordSalt, password);
    } else {
      loginConfig.password = password;
    }

    const CmdLogin = Command_Login.create(loginConfig);

    const command = SessionCommand.create({
      '.Command_Login.ext': CmdLogin,
    });

    webClient.protobuf.sendSessionCommand(command, (raw) => {
      const resp = raw['.Response_Login.ext'];

      if (raw.responseCode === Response.ResponseCode.RespOk) {
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
        case Response.ResponseCode.RespClientUpdateRequired:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: missing features'
          );
          break;

        case Response.ResponseCode.RespWrongPassword:
        case Response.ResponseCode.RespUsernameInvalid:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: incorrect username or password'
          );
          break;

        case Response.ResponseCode.RespWouldOverwriteOldSession:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: duplicated user session'
          );
          break;

        case Response.ResponseCode.RespUserIsBanned:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: banned user'
          );
          break;

        case Response.ResponseCode.RespRegistrationRequired:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: registration required'
          );
          break;

        case Response.ResponseCode.RespClientIdRequired:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: missing client ID'
          );
          break;

        case Response.ResponseCode.RespContextError:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: server error'
          );
          break;

        case Response.ResponseCode.RespAccountNotActivated:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: account not activated'
          );
          SessionPersistence.accountAwaitingActivation(options);
          break;

        default:
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            `Login failed: unknown error: ${raw.responseCode}`
          );
      }

      SessionPersistence.loginFailed();
      SessionCommands.disconnect();
    });
  }

  static requestPasswordSalt(options: WebSocketConnectOptions): void {
    const { userName } = options as RequestPasswordSaltParams;

    const registerConfig = {
      ...webClient.clientConfig,
      userName,
    };

    const CmdRequestPasswordSalt =
      Command_RequestPasswordSalt.create(registerConfig);

    const sc = SessionCommand.create({
      '.Command_RequestPasswordSalt.ext': CmdRequestPasswordSalt,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      switch (raw.responseCode) {
        case Response.ResponseCode.RespOk: {
          const passwordSalt = raw['.Response_PasswordSalt.ext']?.passwordSalt;

          switch (options.reason) {
            case WebSocketConnectReason.ACTIVATE_ACCOUNT: {
              SessionCommands.activateAccount(options, passwordSalt);
              break;
            }

            case WebSocketConnectReason.PASSWORD_RESET: {
              SessionCommands.resetPassword(options, passwordSalt);
              break;
            }

            case WebSocketConnectReason.LOGIN:
            default: {
              SessionCommands.login(options, passwordSalt);
            }
          }

          return;
        }
        case Response.ResponseCode.RespRegistrationRequired: {
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: registration required'
          );
          break;
        }
        default: {
          SessionCommands.updateStatus(
            StatusEnum.DISCONNECTED,
            'Login failed: Unknown Reason'
          );
        }
      }

      switch (options.reason) {
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

  static register(
    options: WebSocketConnectOptions,
    passwordSalt: string | null
  ): void {
    const { userName, password, email, country, realName } =
      options as ServerRegisterParams;

    const registerConfig: any = {
      ...webClient.clientConfig,
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

    const CmdRegister = Command_Register.create(registerConfig);

    const sc = SessionCommand.create({
      '.Command_Register.ext': CmdRegister,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      if (raw.responseCode === Response.ResponseCode.RespRegistrationAccepted) {
        SessionCommands.login(options, passwordSalt);
        SessionPersistence.registrationSuccess();
        return;
      }

      switch (raw.responseCode) {
        case Response.ResponseCode.RespRegistrationAcceptedNeedsActivation:
          SessionPersistence.accountAwaitingActivation(options);
          break;
        case Response.ResponseCode.RespUserAlreadyExists:
          SessionPersistence.registrationUserNameError('Username is taken');
          break;
        case Response.ResponseCode.RespUsernameInvalid:
          console.error('ResponseCode.RespUsernameInvalid', raw.reasonStr);
          SessionPersistence.registrationUserNameError('Invalid username');
          break;
        case Response.ResponseCode.RespPasswordTooShort:
          SessionPersistence.registrationPasswordError(
            'Your password was too short'
          );
          break;
        case Response.ResponseCode.RespEmailRequiredToRegister:
          SessionPersistence.registrationRequiresEmail();
          break;
        case Response.ResponseCode.RespEmailBlackListed:
          SessionPersistence.registrationEmailError(
            'This email provider has been blocked'
          );
          break;
        case Response.ResponseCode.RespTooManyRequests:
          SessionPersistence.registrationEmailError(
            'Max accounts reached for this email'
          );
          break;
        case Response.ResponseCode.RespRegistrationDisabled:
          SessionPersistence.registrationFailed(
            'Registration is currently disabled'
          );
          break;
        case Response.ResponseCode.RespUserIsBanned:
          SessionPersistence.registrationFailed(
            NormalizeService.normalizeBannedUserError(
              raw.reasonStr,
              raw.endTime
            )
          );
          break;
        case Response.ResponseCode.RespRegistrationFailed:
        default:
          SessionPersistence.registrationFailed(
            'Registration failed due to a server issue'
          );
          break;
      }

      SessionCommands.disconnect();
    });
  }

  static activateAccount(
    options: WebSocketConnectOptions,
    passwordSalt?: string
  ): void {
    const { userName, token } = options as unknown as AccountActivationParams;

    const accountActivationConfig = {
      ...webClient.clientConfig,
      userName,
      token,
    };

    const CmdActivate = Command_Activate.create(accountActivationConfig);

    const sc = SessionCommand.create({
      '.Command_Activate.ext': CmdActivate,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      if (raw.responseCode === Response.ResponseCode.RespActivationAccepted) {
        SessionPersistence.accountActivationSuccess();
        SessionCommands.login(options, passwordSalt);
      } else {
        SessionCommands.updateStatus(
          StatusEnum.DISCONNECTED,
          'Account Activation Failed'
        );
        SessionCommands.disconnect();
        SessionPersistence.accountActivationFailed();
      }
    });
  }

  static resetPasswordRequest(options: WebSocketConnectOptions): void {
    const { userName } = options as unknown as ForgotPasswordParams;

    const forgotPasswordConfig = {
      ...webClient.clientConfig,
      userName,
    };

    const CmdForgotPasswordRequest =
      Command_ForgotPasswordRequest.create(forgotPasswordConfig);

    const sc = SessionCommand.create({
      '.Command_ForgotPasswordRequest.ext': CmdForgotPasswordRequest,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      if (raw.responseCode === Response.ResponseCode.RespOk) {
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

  static resetPasswordChallenge(options: WebSocketConnectOptions): void {
    const { userName, email } =
      options as unknown as ForgotPasswordChallengeParams;

    const forgotPasswordChallengeConfig = {
      ...webClient.clientConfig,
      userName,
      email,
    };

    const CmdForgotPasswordChallenge = Command_ForgotPasswordChallenge.create(
      forgotPasswordChallengeConfig
    );

    const sc = SessionCommand.create({
      '.Command_ForgotPasswordChallenge.ext': CmdForgotPasswordChallenge,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      if (raw.responseCode === Response.ResponseCode.RespOk) {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPassword();
      } else {
        SessionCommands.updateStatus(StatusEnum.DISCONNECTED, null);
        SessionPersistence.resetPasswordFailed();
      }

      SessionCommands.disconnect();
    });
  }

  static resetPassword(
    options: WebSocketConnectOptions,
    passwordSalt?: string
  ): void {
    const { userName, token, newPassword } =
      options as unknown as ForgotPasswordResetParams;

    const forgotPasswordResetConfig: any = {
      ...webClient.clientConfig,
      userName,
      token,
    };

    if (passwordSalt) {
      forgotPasswordResetConfig.hashedNewPassword = hashPassword(
        passwordSalt,
        newPassword
      );
    } else {
      forgotPasswordResetConfig.newPassword = newPassword;
    }

    const CmdForgotPasswordReset = Command_ForgotPasswordReset.create(
      forgotPasswordResetConfig
    );

    const sc = SessionCommand.create({
      '.Command_ForgotPasswordReset.ext': CmdForgotPasswordReset,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      if (raw.responseCode === Response.ResponseCode.RespOk) {
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
    const CmdListUsers = Command_ListUsers.create();

    const sc = SessionCommand.create({
      '.Command_ListUsers.ext': CmdListUsers,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;
      const response = raw['.Response_ListUsers.ext'];

      if (response) {
        switch (responseCode) {
          case Response.ResponseCode.RespOk:
            SessionPersistence.updateUsers(response.userList);
            break;
          default:
            console.log(
              `Failed to fetch Server Rooms [${responseCode}] : `,
              raw
            );
        }
      }
    });
  }

  static listRooms(): void {
    const CmdListRooms = Command_ListRooms.create();

    const sc = SessionCommand.create({
      '.Command_ListRooms.ext': CmdListRooms,
    });

    webClient.protobuf.sendSessionCommand(sc);
  }

  static joinRoom(roomId: number): void {
    const CmdJoinRoom = Command_JoinRoom.create({ roomId });

    const sc = SessionCommand.create({
      '.Command_JoinRoom.ext': CmdJoinRoom,
    });

    webClient.protobuf.sendSessionCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch (responseCode) {
        case Response.ResponseCode.RespOk:
          const { roomInfo } = raw['.Response_JoinRoom.ext'];

          RoomPersistence.joinRoom(roomInfo);
          return;
        case Response.ResponseCode.RespNameNotFound:
          error = "Failed to join the room: it doesn't exist on the server.";
          break;
        case Response.ResponseCode.RespContextError:
          error =
            'The server thinks you are in the room but Cockatrice is unable to display it. Try restarting Cockatrice.';
          break;
        case Response.ResponseCode.RespUserLevelTooLow:
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
    const CmdAddToList = Command_AddToList.create({ list, userName });

    const sc = SessionCommand.create({
      '.Command_AddToList.ext': CmdAddToList,
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static removeFromList(list: string, userName: string): void {
    const CmdRemoveFromList = Command_RemoveFromList.create({ list, userName });

    const sc = SessionCommand.create({
      '.Command_RemoveFromList.ext': CmdRemoveFromList,
    });

    webClient.protobuf.sendSessionCommand(sc, ({ responseCode }) => {
      // @TODO: filter responseCode, pop snackbar for error
    });
  }

  static viewLogHistory(filters): void {
    const CmdViewLogHistory = Command_ViewLogHistory.create(filters);

    const sc = ModeratorCommand.create({
      '.Command_ViewLogHistory.ext': CmdViewLogHistory,
    });

    webClient.protobuf.sendModeratorCommand(sc, (raw) => {
      const { responseCode } = raw;

      let error;

      switch (responseCode) {
        case Response.ResponseCode.RespOk:
          const { logMessage } = raw['.Response_ViewLogHistory.ext'];
          SessionPersistence.viewLogs(logMessage);
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
