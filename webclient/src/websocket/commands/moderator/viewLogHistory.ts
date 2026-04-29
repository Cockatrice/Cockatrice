import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ViewLogHistory_ext, Command_ViewLogHistorySchema, Response_ViewLogHistory_ext } from '@app/generated';
import type { ViewLogHistoryParams } from '@app/generated';

export function viewLogHistory(filters: ViewLogHistoryParams): void {
  WebClient.instance.protobuf.sendModeratorCommand(Command_ViewLogHistory_ext, create(Command_ViewLogHistorySchema, filters), {
    responseExt: Response_ViewLogHistory_ext,
    onSuccess: (response) => {
      WebClient.instance.response.moderator.viewLogs(response.logMessage);
    },
  });
}
