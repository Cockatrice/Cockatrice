import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';

import { Data } from '@app/types';

export function viewLogHistory(filters: Data.ViewLogHistoryParams): void {
  webClient.protobuf.sendModeratorCommand(Data.Command_ViewLogHistory_ext, create(Data.Command_ViewLogHistorySchema, filters), {
    responseExt: Data.Response_ViewLogHistory_ext,
    onSuccess: (response) => {
      ModeratorPersistence.viewLogs(response.logMessage);
    },
  });
}
