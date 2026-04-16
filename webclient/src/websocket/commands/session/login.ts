import { App, Enriched, Data } from '@app/types';
import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { hashPassword } from '../../utils';
import {
  disconnect,
  listUsers,
  listRooms,
  updateStatus,
} from './';

export function login(options: Omit<Enriched.LoginConnectOptions, 'password'>, password?: string, passwordSalt?: string): void {
  const { userName, hashedPassword } = options;

  const loginConfig = {
    ...CLIENT_CONFIG,
    clientid: 'webatrice',
    userName,
    ...(passwordSalt
      ? { hashedPassword: hashedPassword || hashPassword(passwordSalt, password) }
      : { password }),
  } satisfies MessageInitShape<typeof Data.Command_LoginSchema>;

  const onLoginError = (message: string, extra?: () => void) => {
    updateStatus(App.StatusEnum.DISCONNECTED, message);
    extra?.();
    WebClient.instance.response.session.loginFailed();
    disconnect();
  };

  WebClient.instance.protobuf.sendSessionCommand(Data.Command_Login_ext, create(Data.Command_LoginSchema, loginConfig), {
    responseExt: Data.Response_Login_ext,
    onSuccess: (resp) => {
      const { buddyList, ignoreList, userInfo } = resp;

      WebClient.instance.response.session.updateBuddyList(buddyList);
      WebClient.instance.response.session.updateIgnoreList(ignoreList);
      WebClient.instance.response.session.updateUser(userInfo);
      WebClient.instance.response.session.loginSuccessful({ hashedPassword: loginConfig.hashedPassword });

      listUsers();
      listRooms();

      updateStatus(App.StatusEnum.LOGGED_IN, 'Logged in.');
    },
    onResponseCode: {
      [Data.Response_ResponseCode.RespClientUpdateRequired]: () =>
        onLoginError('Login failed: missing features'),
      [Data.Response_ResponseCode.RespWrongPassword]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [Data.Response_ResponseCode.RespUsernameInvalid]: () =>
        onLoginError('Login failed: incorrect username or password'),
      [Data.Response_ResponseCode.RespWouldOverwriteOldSession]: () =>
        onLoginError('Login failed: duplicated user session'),
      [Data.Response_ResponseCode.RespUserIsBanned]: () =>
        onLoginError('Login failed: banned user'),
      [Data.Response_ResponseCode.RespRegistrationRequired]: () =>
        onLoginError('Login failed: registration required'),
      [Data.Response_ResponseCode.RespClientIdRequired]: () =>
        onLoginError('Login failed: missing client ID'),
      [Data.Response_ResponseCode.RespContextError]: () =>
        onLoginError('Login failed: server error'),
      [Data.Response_ResponseCode.RespAccountNotActivated]: () =>
        onLoginError('Login failed: account not activated',
          () => {
            WebClient.instance.response.session.accountAwaitingActivation({
              host: options.host,
              port: options.port,
              userName: options.userName,
            });
          }
        ),
    },
    onError: (responseCode) =>
      onLoginError(`Login failed: unknown error: ${responseCode}`),
  });
}
