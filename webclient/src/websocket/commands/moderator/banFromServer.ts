import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_BanFromServer_ext, create(Data.Command_BanFromServerSchema, {
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  }), {
    onSuccess: () => {
      WebClient.instance.response.moderator.banFromServer(userName);
    },
  });
}
