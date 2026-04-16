import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetBanHistory_ext, Command_GetBanHistorySchema, Response_BanHistory_ext } from '@app/generated';

export function getBanHistory(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(Command_GetBanHistory_ext, create(Command_GetBanHistorySchema, { userName }), {
    responseExt: Response_BanHistory_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.banHistory(userName, response.banList);
    },
  });
}
