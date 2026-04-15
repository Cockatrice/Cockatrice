import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { ModeratorPersistence } from '../../persistence';
import { Data } from '@app/types';

export function getWarnList(modName: string, userName: string, userClientid: string): void {
  webClient.protobuf.sendModeratorCommand(
    Data.Command_GetWarnList_ext,
    create(Data.Command_GetWarnListSchema, { modName, userName, userClientid }),
    {
      responseExt: Data.Response_WarnList_ext,
      onSuccess: (response) => {
        ModeratorPersistence.warnListOptions([response]);
      },
    }
  );
}
