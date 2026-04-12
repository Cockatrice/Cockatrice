import { StatusEnum, WebSocketConnectOptions } from 'types';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { ProtoController } from '../../services/ProtoController';
import { hashPassword } from '../../utils';
import { SessionPersistence } from '../../persistence';

import {
  disconnect,
  listUsers,
  listRooms,
  updateStatus,
} from './';

export function login(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, password, hashedPassword } = options;

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

  const { ResponseCode } = ProtoController.root.Response;

  const onLoginError = (message: string, extra?: () => void) => {
    updateStatus(StatusEnum.DISCONNECTED, message);
    extra?.();
    SessionPersistence.loginFailed();
    disconnect();
  };

  BackendService.sendSessionCommand('Command_Login', loginConfig, {
    responseName: 'Response_Login',
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
      [ResponseCode.RespClientUpdateRequired]: () =>
        onLoginError('Login failed: missing features'),
      [ResponseCode.RespWrongPassword]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [ResponseCode.RespUsernameInvalid]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [ResponseCode.RespWouldOverwriteOldSession]: () =>
        onLoginError('Login failed: duplicated user session'),
      [ResponseCode.RespUserIsBanned]: () =>
        onLoginError('Login failed: banned user'),
      [ResponseCode.RespRegistrationRequired]: () =>
        onLoginError('Login failed: registration required'),
      [ResponseCode.RespClientIdRequired]: () =>
        onLoginError('Login failed: missing client ID'),
      [ResponseCode.RespContextError]: () =>
        onLoginError('Login failed: server error'),
      [ResponseCode.RespAccountNotActivated]: () =>
        onLoginError('Login failed: account not activated',
          () => SessionPersistence.accountAwaitingActivation(options)
        ),
    },
    onError: (responseCode) =>
      onLoginError(`Login failed: unknown error: ${responseCode}`),
  });
}
