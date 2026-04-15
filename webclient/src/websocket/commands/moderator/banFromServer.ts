import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  webClient.protobuf.sendModeratorCommand(Data.Command_BanFromServer_ext, create(Data.Command_BanFromServerSchema, {
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  }), {
    onSuccess: () => {
      ModeratorPersistence.banFromServer(userName);
    },
  });
}
