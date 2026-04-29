import { create } from '@bufbuild/protobuf';
import {
  Command_RequestPasswordSalt_ext,
  Command_RequestPasswordSaltSchema,
  Response_PasswordSalt_ext,
  Response_ResponseCode,
  type RequestPasswordSaltParams,
} from '@app/generated';

import { StatusEnum } from '../../types/StatusEnum';
import { CLIENT_CONFIG } from '../../config';
import { WebClient } from '../../WebClient';
import type { ConnectTarget } from '../../types/WebClientConfig';
import { updateStatus } from './';

export function requestPasswordSalt(
  options: ConnectTarget & RequestPasswordSaltParams,
  onSaltReceived: (passwordSalt: string) => void,
  onFailure: () => void,
): void {
  const { userName } = options;

  WebClient.instance.protobuf.sendSessionCommand(Command_RequestPasswordSalt_ext, create(Command_RequestPasswordSaltSchema, {
    ...CLIENT_CONFIG,
    userName,
  }), {
    responseExt: Response_PasswordSalt_ext,
    onSuccess: (resp) => {
      onSaltReceived(resp?.passwordSalt);
    },
    onResponseCode: {
      [Response_ResponseCode.RespRegistrationRequired]: () => {
        updateStatus(StatusEnum.DISCONNECTED, 'Login failed: registration required');
        onFailure();
      },
    },
    onError: () => {
      updateStatus(StatusEnum.DISCONNECTED, 'Login failed: Unknown Reason');
      onFailure();
    },
  });
}
