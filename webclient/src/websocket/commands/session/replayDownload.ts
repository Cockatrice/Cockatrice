import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function replayDownload(replayId: number): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Data.Command_ReplayDownload_ext,
    create(Data.Command_ReplayDownloadSchema, { replayId }),
    {
      responseExt: Data.Response_ReplayDownload_ext,
      onSuccess: (response) => {
        WebClient.instance.response.session.replayDownloaded(replayId, response);
      },
    }
  );
}
