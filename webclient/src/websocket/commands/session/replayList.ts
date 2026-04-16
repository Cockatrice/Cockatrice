import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function replayList(): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_ReplayList_ext, create(Data.Command_ReplayListSchema), {
    responseExt: Data.Response_ReplayList_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.replayList(response.matchList);
    },
  });
}
