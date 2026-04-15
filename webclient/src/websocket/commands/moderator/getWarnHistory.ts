import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getWarnHistory(userName: string): void {
  webClient.protobuf.sendModeratorCommand(Data.Command_GetWarnHistory_ext, create(Data.Command_GetWarnHistorySchema, { userName }), {
    responseExt: Data.Response_WarnHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.warnHistory(userName, response.warnList);
    },
  });
}
