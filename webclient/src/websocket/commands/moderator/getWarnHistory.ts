import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetWarnHistory_ext, Command_GetWarnHistorySchema, Response_WarnHistory_ext } from '@app/generated';

export function getWarnHistory(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Command_GetWarnHistory_ext,
    create(Command_GetWarnHistorySchema, { userName }),
    {
      responseExt: Response_WarnHistory_ext,
      onSuccess: (response) => {
        WebClient.instance.response.moderator.warnHistory(userName, response.warnList);
      },
    }
  );
}
