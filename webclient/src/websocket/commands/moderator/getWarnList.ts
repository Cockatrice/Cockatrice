import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_GetWarnList_ext, Command_GetWarnListSchema, Response_WarnList_ext } from '@app/generated';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  WebClient.instance.protobuf.sendModeratorCommand(
    Command_GetWarnList_ext,
    create(Command_GetWarnListSchema, { modName, userName, userClientid }),
    {
      responseExt: Response_WarnList_ext,
      onSuccess: (response) => {
        WebClient.instance.response.moderator.warnListOptions([response]);
      },
    }
  );
}
