import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Data.Command_GetWarnList_ext,
    create(Data.Command_GetWarnListSchema, { modName, userName, userClientid }),
    {
      responseExt: Data.Response_WarnList_ext,
      onSuccess: (response) => {
        WebClient.instance.response.moderator.warnListOptions([response]);
      },
    }
  );
}
