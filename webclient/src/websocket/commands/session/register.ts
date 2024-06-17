import { ServerRegisterParams } from 'store';
import { WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { SessionPersistence } from '../../persistence';
import { hashPassword } from '../../utils';
import NormalizeService from '../../utils/NormalizeService';

import { login, disconnect } from './';

export function register(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, password, email, country, realName } = options as ServerRegisterParams;

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

  const command = webClient.protobuf.controller.Command_Register.create(registerConfig);
  const sc = webClient.protobuf.controller.SessionCommand.create({ '.Command_Register.ext': command });

  webClient.protobuf.sendSessionCommand(sc, raw => {
    if (raw.responseCode === webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAccepted) {
      login(options, passwordSalt);
      SessionPersistence.registrationSuccess()
      return;
    }

    switch (raw.responseCode) {
      case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationAcceptedNeedsActivation:
        SessionPersistence.accountAwaitingActivation(options);
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespUserAlreadyExists:
        SessionPersistence.registrationUserNameError('Username is taken');
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespUsernameInvalid:
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
        SessionPersistence.registrationFailed(raw.reasonStr, raw.endTime);
        break;
      case webClient.protobuf.controller.Response.ResponseCode.RespRegistrationFailed:
      default:
        SessionPersistence.registrationFailed('Registration failed due to a server issue');
        break;
    }

    disconnect();
  });
}
