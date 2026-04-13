import { AccountActivationParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { Command_Activate_ext, Command_ActivateSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';
import { Response_ResponseCode } from 'generated/proto/response_pb';

import { disconnect, login, updateStatus } from './';

export function activate(options: WebSocketConnectOptions, password?: string, passwordSalt?: string): void {
  const { userName, token } = options as unknown as AccountActivationParams;

  BackendService.sendSessionCommand(Command_Activate_ext, create(Command_ActivateSchema, {
    ...webClient.clientConfig,
    userName,
    token,
  }), {
    onResponseCode: {
      [Response_ResponseCode.RespActivationAccepted]: () => {
        SessionPersistence.accountActivationSuccess();
        login(options, password, passwordSalt);
      },
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Account Activation Failed');
      disconnect();
      SessionPersistence.accountActivationFailed();
    },
  });
}
