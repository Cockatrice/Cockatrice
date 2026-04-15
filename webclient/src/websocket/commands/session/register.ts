import { ServerRegisterParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { ProtoController } from '../../services/ProtoController';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';

import { login, disconnect, updateStatus } from './';

export function register(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, password, email, country, realName } = options as ServerRegisterParams;

  const params: any = {
    ...webClient.clientConfig,
    userName,
    email,
    country,
    realName,
  };

  if (passwordSalt) {
    params.hashedPassword = hashPassword(passwordSalt, password);
  } else {
    params.password = password;
  }

  const { ResponseCode } = ProtoController.root.Response;

  const onRegistrationError = (action: () => void) => {
    action();
    updateStatus(StatusEnum.DISCONNECTED, 'Registration failed');
    disconnect();
  };

  BackendService.sendSessionCommand('Command_Register', params, {
    onResponseCode: {
      [ResponseCode.RespRegistrationAccepted]: () => {
        login(options, passwordSalt);
        SessionPersistence.registrationSuccess();
      },
      [ResponseCode.RespRegistrationAcceptedNeedsActivation]: () => {
        updateStatus(StatusEnum.DISCONNECTED, 'Registration accepted, awaiting activation');
        SessionPersistence.accountAwaitingActivation(options);
        disconnect();
      },
      [ResponseCode.RespUserAlreadyExists]: () => onRegistrationError(
        () => SessionPersistence.registrationUserNameError('Username is taken')
      ),
      [ResponseCode.RespUsernameInvalid]: () => onRegistrationError(
        () => SessionPersistence.registrationUserNameError('Invalid username')
      ),
      [ResponseCode.RespPasswordTooShort]: () => onRegistrationError(
        () => SessionPersistence.registrationPasswordError('Your password was too short')
      ),
      [ResponseCode.RespEmailRequiredToRegister]: () => onRegistrationError(
        () => SessionPersistence.registrationRequiresEmail()
      ),
      [ResponseCode.RespEmailBlackListed]: () => onRegistrationError(
        () => SessionPersistence.registrationEmailError('This email provider has been blocked')
      ),
      [ResponseCode.RespTooManyRequests]: () => onRegistrationError(
        () => SessionPersistence.registrationEmailError('Max accounts reached for this email')
      ),
      [ResponseCode.RespRegistrationDisabled]: () => onRegistrationError(
        () => SessionPersistence.registrationFailed('Registration is currently disabled')
      ),
      [ResponseCode.RespUserIsBanned]: (raw) => onRegistrationError(
        () => SessionPersistence.registrationFailed(raw.reasonStr, raw.endTime)
      ),
    },
    onError: () => onRegistrationError(
      () => SessionPersistence.registrationFailed('Registration failed due to a server issue')
    ),
  });
}
