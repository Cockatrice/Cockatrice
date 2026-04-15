import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function replayList(): void {
  webClient.protobuf.sendSessionCommand(Data.Command_ReplayList_ext, create(Data.Command_ReplayListSchema), {
    responseExt: Data.Response_ReplayList_ext,
    onSuccess: (response) => {
      SessionPersistence.replayList(response.matchList);
    },
  });
}
