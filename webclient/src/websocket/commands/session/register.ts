import { App, Enriched, Data } from '@app/types';

import { create, getExtension } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { hashPassword } from '../../utils';

import { login, disconnect, updateStatus } from './';

export function register(options: Omit<Enriched.RegisterConnectOptions, 'password'>, password?: string, passwordSalt?: string): void {
  const { userName, email, country, realName } = options;

  const params: MessageInitShape<typeof Data.Command_RegisterSchema> = {
    ...CLIENT_CONFIG,
    userName,
    email,
    country,
    realName,
    ...(passwordSalt
      ? { hashedPassword: hashPassword(passwordSalt, password) }
      : { password }),
  };

  const onRegistrationError = (action: () => void) => {
    action();
    updateStatus(App.StatusEnum.DISCONNECTED, 'Registration failed');
    disconnect();
  };

  WebClient.instance.protobuf.sendSessionCommand(Data.Command_Register_ext, create(Data.Command_RegisterSchema, params), {
    onResponseCode: {
      [Data.Response_ResponseCode.RespRegistrationAccepted]: () => {
        login({
          host: options.host,
          port: options.port,
          userName: options.userName,
          reason: App.WebSocketConnectReason.LOGIN,
        }, password, passwordSalt);
        WebClient.instance.response.session.registrationSuccess();
      },
      [Data.Response_ResponseCode.RespRegistrationAcceptedNeedsActivation]: () => {
        updateStatus(App.StatusEnum.DISCONNECTED, 'Registration accepted, awaiting activation');
        WebClient.instance.response.session.accountAwaitingActivation({
          host: options.host,
          port: options.port,
          userName: options.userName,
        });
        disconnect();
      },
      [Data.Response_ResponseCode.RespUserAlreadyExists]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationUserNameError('Username is taken')
      ),
      [Data.Response_ResponseCode.RespUsernameInvalid]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationUserNameError('Invalid username')
      ),
      [Data.Response_ResponseCode.RespPasswordTooShort]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationPasswordError('Your password was too short')
      ),
      [Data.Response_ResponseCode.RespEmailRequiredToRegister]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationRequiresEmail()
      ),
      [Data.Response_ResponseCode.RespEmailBlackListed]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationEmailError('This email provider has been blocked')
      ),
      [Data.Response_ResponseCode.RespTooManyRequests]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationEmailError('Max accounts reached for this email')
      ),
      [Data.Response_ResponseCode.RespRegistrationDisabled]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationFailed('Registration is currently disabled')
      ),
      [Data.Response_ResponseCode.RespUserIsBanned]: (raw) => {
        const register = getExtension(raw, Data.Response_Register_ext);
        onRegistrationError(
          () => WebClient.instance.response.session.registrationFailed(register.deniedReasonStr, Number(register.deniedEndTime))
        );
      },
    },
    onError: () => onRegistrationError(
      () => WebClient.instance.response.session.registrationFailed('Registration failed due to a server issue')
    ),
  });
}
