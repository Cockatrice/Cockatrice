import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DeckDelDirSchema, Command_DeckDelDir_ext } from 'generated/proto/command_deck_del_dir_pb';
import { SessionPersistence } from '../../persistence';

export function deckDelDir(path: string): void {
  BackendService.sendSessionCommand(Command_DeckDelDir_ext, create(Command_DeckDelDirSchema, { path }), {
    onSuccess: () => {
      SessionPersistence.deleteServerDeckDir(path);
    },
  });
}
