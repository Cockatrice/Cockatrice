import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function deckDelDir(path: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_DeckDelDir_ext, create(Data.Command_DeckDelDirSchema, { path }), {
    onSuccess: () => {
      WebClient.instance.response.session.deleteServerDeckDir(path);
    },
  });
}
