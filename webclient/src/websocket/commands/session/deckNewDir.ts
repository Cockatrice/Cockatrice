import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckNewDir_ext, Command_DeckNewDirSchema } from '@app/generated';

export function deckNewDir(path: string, dirName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_DeckNewDir_ext, create(Command_DeckNewDirSchema, { path, dirName }), {
    onSuccess: () => {
      WebClient.instance.response.session.createServerDeckDir(path, dirName);
    },
  });
}
