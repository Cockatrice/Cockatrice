import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getBanHistory(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_GetBanHistory_ext, create(Data.Command_GetBanHistorySchema, { userName }), {
    responseExt: Data.Response_BanHistory_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.banHistory(userName, response.banList);
    },
  });
}
