import { StatusEnum, WebSocketConnectOptions } from 'types';
import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { Command_Login_ext, Command_LoginSchema } from 'generated/proto/session_commands_pb';
import { hashPassword } from '../../utils';
import { SessionPersistence } from '../../persistence';
import { Response_Login_ext } from 'generated/proto/response_login_pb';
import { Response_ResponseCode } from 'generated/proto/response_pb';

import {
  disconnect,
  listUsers,
  listRooms,
  updateStatus,
} from './';

export function login(options: WebSocketConnectOptions, password?: string, passwordSalt?: string): void {
  const { userName, hashedPassword } = options;

  const loginConfig: MessageInitShape<typeof Command_LoginSchema> = {
    ...webClient.clientConfig,
    clientid: 'webatrice',
    userName,
    ...(passwordSalt
      ? { hashedPassword: hashedPassword || hashPassword(passwordSalt, password) }
      : { password }),
  };

  const onLoginError = (message: string, extra?: () => void) => {
    updateStatus(StatusEnum.DISCONNECTED, message);
    extra?.();
    SessionPersistence.loginFailed();
    disconnect();
  };

  BackendService.sendSessionCommand(Command_Login_ext, create(Command_LoginSchema, loginConfig), {
    responseExt: Response_Login_ext,
    onSuccess: (resp) => {
      const { buddyList, ignoreList, userInfo } = resp;

      SessionPersistence.updateBuddyList(buddyList);
      SessionPersistence.updateIgnoreList(ignoreList);
      SessionPersistence.updateUser(userInfo);
      const { password: _password, ...safeConfig } = loginConfig;
      SessionPersistence.loginSuccessful(safeConfig);

      listUsers();
      listRooms();

      updateStatus(StatusEnum.LOGGED_IN, 'Logged in.');
    },
    onResponseCode: {
      [Response_ResponseCode.RespClientUpdateRequired]: () =>
        onLoginError('Login failed: missing features'),
      [Response_ResponseCode.RespWrongPassword]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [Response_ResponseCode.RespUsernameInvalid]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [Response_ResponseCode.RespWouldOverwriteOldSession]: () =>
        onLoginError('Login failed: duplicated user session'),
      [Response_ResponseCode.RespUserIsBanned]: () =>
        onLoginError('Login failed: banned user'),
      [Response_ResponseCode.RespRegistrationRequired]: () =>
        onLoginError('Login failed: registration required'),
      [Response_ResponseCode.RespClientIdRequired]: () =>
        onLoginError('Login failed: missing client ID'),
      [Response_ResponseCode.RespContextError]: () =>
        onLoginError('Login failed: server error'),
      [Response_ResponseCode.RespAccountNotActivated]: () =>
        onLoginError('Login failed: account not activated',
          () => {
            const { password: _p, newPassword: _np, ...safeOptions } = options;
            SessionPersistence.accountAwaitingActivation(safeOptions);
          }
        ),
    },
    onError: (responseCode) =>
      onLoginError(`Login failed: unknown error: ${responseCode}`),
  });
}
