import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function deckNewDir(path: string, dirName: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_DeckNewDir_ext, create(Data.Command_DeckNewDirSchema, { path, dirName }), {
    onSuccess: () => {
      SessionPersistence.createServerDeckDir(path, dirName);
    },
  });
}
