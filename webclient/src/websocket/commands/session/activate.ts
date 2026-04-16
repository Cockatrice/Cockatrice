import { App, Enriched, Data } from '@app/types';

import { create } from '@bufbuild/protobuf';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';

import { disconnect, login, updateStatus } from './';

export function activate(options: Omit<Enriched.ActivateConnectOptions, 'password'>, password?: string, passwordSalt?: string): void {
  const { userName, token } = options;

  WebClient.instance.protobuf.sendSessionCommand(Data.Command_Activate_ext, create(Data.Command_ActivateSchema, {
    ...CLIENT_CONFIG,
    userName,
    token,
  }), {
    onResponseCode: {
      [Data.Response_ResponseCode.RespActivationAccepted]: () => {
        WebClient.instance.response.session.accountActivationSuccess();
        login({
          host: options.host,
          port: options.port,
          userName: options.userName,
          reason: App.WebSocketConnectReason.LOGIN,
        }, password, passwordSalt);
      },
    },
    onError: () => {
      updateStatus(App.StatusEnum.DISCONNECTED, 'Account Activation Failed');
      disconnect();
      WebClient.instance.response.session.accountActivationFailed();
    },
  });
}
