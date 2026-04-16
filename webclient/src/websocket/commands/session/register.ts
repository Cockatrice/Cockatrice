import { create, getExtension } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import {
  Command_Register_ext,
  Command_RegisterSchema,
  Response_Register_ext,
  Response_ResponseCode,
  type RegisterParams,
} from '@app/generated';

import { StatusEnum } from '../../StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../WebClientConfig';
import { hashPassword } from '../../utils';
import { login, disconnect, updateStatus } from './';

export function register(options: ConnectTarget & RegisterParams, password?: string, passwordSalt?: string): void {
  const { userName, email, country, realName } = options;

  const params: MessageInitShape<typeof Command_RegisterSchema> = {
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
    updateStatus(StatusEnum.DISCONNECTED, 'Registration failed');
    disconnect();
  };

  WebClient.instance.protobuf.sendSessionCommand(Command_Register_ext, create(Command_RegisterSchema, params), {
    onResponseCode: {
      [Response_ResponseCode.RespRegistrationAccepted]: () => {
        login({
          host: options.host,
          port: options.port,
          userName: options.userName,
        }, password, passwordSalt);
        WebClient.instance.response.session.registrationSuccess();
      },
      [Response_ResponseCode.RespRegistrationAcceptedNeedsActivation]: (raw) => {
        updateStatus(StatusEnum.DISCONNECTED, 'Registration accepted, awaiting activation');
        WebClient.instance.response.session.accountAwaitingActivation({
          ...raw,
          host: options.host,
          port: options.port,
          userName: options.userName,
        });
        disconnect();
      },
      [Response_ResponseCode.RespUserAlreadyExists]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationUserNameError('Username is taken')
      ),
      [Response_ResponseCode.RespUsernameInvalid]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationUserNameError('Invalid username')
      ),
      [Response_ResponseCode.RespPasswordTooShort]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationPasswordError('Your password was too short')
      ),
      [Response_ResponseCode.RespEmailRequiredToRegister]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationRequiresEmail()
      ),
      [Response_ResponseCode.RespEmailBlackListed]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationEmailError('This email provider has been blocked')
      ),
      [Response_ResponseCode.RespTooManyRequests]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationEmailError('Max accounts reached for this email')
      ),
      [Response_ResponseCode.RespRegistrationDisabled]: () => onRegistrationError(
        () => WebClient.instance.response.session.registrationFailed('Registration is currently disabled')
      ),
      [Response_ResponseCode.RespUserIsBanned]: (raw) => {
        const register = getExtension(raw, Response_Register_ext);
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
