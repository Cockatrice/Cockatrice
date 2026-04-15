import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function deckDelDir(path: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_DeckDelDir_ext, create(Data.Command_DeckDelDirSchema, { path }), {
    onSuccess: () => {
      SessionPersistence.deleteServerDeckDir(path);
    },
  });
}
