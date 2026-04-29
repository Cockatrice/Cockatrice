import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ReplayList_ext, Command_ReplayListSchema, Response_ReplayList_ext } from '@app/generated';

export function replayList(): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_ReplayList_ext, create(Command_ReplayListSchema), {
    responseExt: Response_ReplayList_ext,
    onSuccess: (response) => {
      WebClient.instance.response.session.replayList(response.matchList);
    },
  });
}
