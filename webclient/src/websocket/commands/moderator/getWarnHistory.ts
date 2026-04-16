import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getWarnHistory(userName: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Data.Command_GetWarnHistory_ext,
    create(Data.Command_GetWarnHistorySchema, { userName }),
    {
      responseExt: Data.Response_WarnHistory_ext,
      onSuccess: (response) => {
        WebClient.instance.response.moderator.warnHistory(userName, response.warnList);
      },
    }
  );
}
