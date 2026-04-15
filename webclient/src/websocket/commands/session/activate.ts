import { AccountActivationParams } from 'store';
import { StatusEnum, WebSocketConnectOptions } from 'types';

import webClient from '../../WebClient';
import { BackendService } from '../../services/BackendService';
import { ProtoController } from '../../services/ProtoController';
import { SessionPersistence } from '../../persistence';

import { disconnect, login, updateStatus } from './';

export function activate(options: WebSocketConnectOptions, passwordSalt?: string): void {
  const { userName, token } = options as unknown as AccountActivationParams;

  BackendService.sendSessionCommand('Command_Activate', {
    ...webClient.clientConfig,
    userName,
    token,
  }, {
    onResponseCode: {
      [ProtoController.root.Response.ResponseCode.RespActivationAccepted]: () => {
        SessionPersistence.accountActivationSuccess();
        login(options, passwordSalt);
      },
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Account Activation Failed');
      disconnect();
      SessionPersistence.accountActivationFailed();
    },
  });
}
