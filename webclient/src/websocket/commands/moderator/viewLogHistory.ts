import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function viewLogHistory(filters: Data.ViewLogHistoryParams): void {
  WebClient.instance.protobuf.sendModeratorCommand(Data.Command_ViewLogHistory_ext, create(Data.Command_ViewLogHistorySchema, filters), {
    responseExt: Data.Response_ViewLogHistory_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.viewLogs(response.logMessage);
    },
  });
}
