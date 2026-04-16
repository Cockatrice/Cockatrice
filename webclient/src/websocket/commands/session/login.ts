import { create } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import {
  Command_Login_ext,
  Command_LoginSchema,
  Response_Login_ext,
  Response_ResponseCode,
  type LoginParams,
} from '@app/generated';

import { StatusEnum } from '../../StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../WebClientConfig';
import { hashPassword } from '../../utils';
import {
  disconnect,
  listUsers,
  listRooms,
  updateStatus,
} from './';

export function login(options: ConnectTarget & LoginParams, password?: string, passwordSalt?: string): void {
  const { userName, hashedPassword } = options;

  const loginConfig = {
    ...CLIENT_CONFIG,
    clientid: 'webatrice',
    userName,
    ...(passwordSalt
      ? { hashedPassword: hashedPassword || hashPassword(passwordSalt, password) }
      : { password }),
  } satisfies MessageInitShape<typeof Command_LoginSchema>;

  const onLoginError = (message: string, extra?: () => void) => {
    updateStatus(StatusEnum.DISCONNECTED, message);
    extra?.();
    WebClient.instance.response.session.loginFailed();
    disconnect();
  };

  WebClient.instance.protobuf.sendSessionCommand(Command_Login_ext, create(Command_LoginSchema, loginConfig), {
    responseExt: Response_Login_ext,
    onSuccess: (resp) => {
      const { buddyList, ignoreList, userInfo } = resp;

      WebClient.instance.response.session.updateBuddyList(buddyList);
      WebClient.instance.response.session.updateIgnoreList(ignoreList);
      WebClient.instance.response.session.updateUser(userInfo);
      WebClient.instance.response.session.loginSuccessful({ ...resp, hashedPassword: loginConfig.hashedPassword });

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
      [Response_ResponseCode.RespAccountNotActivated]: (raw) =>
        onLoginError('Login failed: account not activated',
          () => {
            WebClient.instance.response.session.accountAwaitingActivation({
              ...raw,
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
