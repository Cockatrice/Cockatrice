import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckNewDir(path: string, dirName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_DeckNewDir_ext, create(Data.Command_DeckNewDirSchema, { path, dirName }), {
    onSuccess: () => {
      WebClient.instance.response.session.createServerDeckDir(path, dirName);
    },
  });
}
