import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_DeckDelDir_ext, Command_DeckDelDirSchema } from '@app/generated';

export function deckDelDir(path: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_DeckDelDir_ext, create(Command_DeckDelDirSchema, { path }), {
    onSuccess: () => {
      WebClient.instance.response.session.deleteServerDeckDir(path);
    },
  });
}
