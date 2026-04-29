import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_ReplayDownload_ext, Command_ReplayDownloadSchema, Response_ReplayDownload_ext } from '@app/generated';

export function replayDownload(replayId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_ReplayDownload_ext,
    create(Command_ReplayDownloadSchema, { replayId }),
    {
      responseExt: Response_ReplayDownload_ext,
      onSuccess: (response) => {
        WebClient.instance.response.session.replayDownloaded(replayId, response);
      },
    }
  );
}
