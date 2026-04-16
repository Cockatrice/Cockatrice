import { create } from '@bufbuild/protobuf';
import {
  Command_Activate_ext,
  Command_ActivateSchema,
  Response_ResponseCode,
  type ActivateParams,
} from '@app/generated';

import { StatusEnum } from '../../interfaces/StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../interfaces/WebClientConfig';
import { disconnect, login, updateStatus } from './';

export function activate(options: ConnectTarget & ActivateParams, password?: string, passwordSalt?: string): void {
  const { userName, token } = options;

  WebClient.instance.protobuf.sendSessionCommand(Command_Activate_ext, create(Command_ActivateSchema, {
    ...CLIENT_CONFIG,
    userName,
    token,
  }), {
    onResponseCode: {
      [Response_ResponseCode.RespActivationAccepted]: () => {
        WebClient.instance.response.session.accountActivationSuccess();
        login({
          host: options.host,
          port: options.port,
          userName: options.userName,
        }, password, passwordSalt);
      },
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Account Activation Failed');
      disconnect();
      WebClient.instance.response.session.accountActivationFailed();
    },
  });
}
