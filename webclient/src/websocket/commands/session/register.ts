import { ServerRegisterParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create, getExtension } from '@bufbuild/protobuf';
import type { MessageInitShape } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { Command_Register_ext, Command_RegisterSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';
import { Response_ResponseCode } from 'generated/proto/response_pb';
import { Response_Register_ext } from 'generated/proto/response_register_pb';

import { login, disconnect, updateStatus } from './';

export function register(options: WebSocketConnectOptions, password?: string, passwordSalt?: string): void {
  const { userName, email, country, realName } = options as ServerRegisterParams;

  const params: MessageInitShape<typeof Command_RegisterSchema> = {
    ...webClient.clientConfig,
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

  BackendService.sendSessionCommand(Command_Register_ext, create(Command_RegisterSchema, params), {
    onResponseCode: {
      [Response_ResponseCode.RespRegistrationAccepted]: () => {
        login(options, password, passwordSalt);
        SessionPersistence.registrationSuccess();
      },
      [Response_ResponseCode.RespRegistrationAcceptedNeedsActivation]: () => {
        updateStatus(StatusEnum.DISCONNECTED, 'Registration accepted, awaiting activation');
        const { password: _p, newPassword: _np, ...safeOptions } = options;
        SessionPersistence.accountAwaitingActivation(safeOptions);
        disconnect();
      },
      [Response_ResponseCode.RespUserAlreadyExists]: () => onRegistrationError(
        () => SessionPersistence.registrationUserNameError('Username is taken')
      ),
      [Response_ResponseCode.RespUsernameInvalid]: () => onRegistrationError(
        () => SessionPersistence.registrationUserNameError('Invalid username')
      ),
      [Response_ResponseCode.RespPasswordTooShort]: () => onRegistrationError(
        () => SessionPersistence.registrationPasswordError('Your password was too short')
      ),
      [Response_ResponseCode.RespEmailRequiredToRegister]: () => onRegistrationError(
        () => SessionPersistence.registrationRequiresEmail()
      ),
      [Response_ResponseCode.RespEmailBlackListed]: () => onRegistrationError(
        () => SessionPersistence.registrationEmailError('This email provider has been blocked')
      ),
      [Response_ResponseCode.RespTooManyRequests]: () => onRegistrationError(
        () => SessionPersistence.registrationEmailError('Max accounts reached for this email')
      ),
      [Response_ResponseCode.RespRegistrationDisabled]: () => onRegistrationError(
        () => SessionPersistence.registrationFailed('Registration is currently disabled')
      ),
      [Response_ResponseCode.RespUserIsBanned]: (raw) => {
        const register = getExtension(raw, Response_Register_ext);
        onRegistrationError(
          () => SessionPersistence.registrationFailed(register.deniedReasonStr, Number(register.deniedEndTime))
        );
      },
    },
    onError: () => onRegistrationError(
      () => SessionPersistence.registrationFailed('Registration failed due to a server issue')
    ),
  });
}
