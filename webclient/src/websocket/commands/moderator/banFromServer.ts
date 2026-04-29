import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_BanFromServer_ext, Command_BanFromServerSchema } from '@app/generated';

export function banFromServer(minutes: number, userName?: string, address?: string, reason?: string,
  visibleReason?: string, clientid?: string, removeMessages?: number): void {
  WebClient.instance.protobuf.sendModeratorCommand(Command_BanFromServer_ext, create(Command_BanFromServerSchema, {
    minutes, userName, address, reason, visibleReason, clientid, removeMessages
  }), {
    onSuccess: () => {
      WebClient.instance.response.moderator.banFromServer(userName);
    },
  });
}
