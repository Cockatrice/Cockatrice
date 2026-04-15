import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getBanHistory(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Data.Command_GetBanHistory_ext, create(Data.Command_GetBanHistorySchema, { userName }), {
    responseExt: Data.Response_BanHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.banHistory(userName, response.banList);
    },
  });
}
