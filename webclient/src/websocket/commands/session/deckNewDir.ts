import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_DeckNewDirSchema, Command_DeckNewDir_ext } from 'generated/proto/command_deck_new_dir_pb';
import { SessionPersistence } from '../../persistence';

export function deckNewDir(path: string, dirName: string): void {
  BackendService.sendSessionCommand(Command_DeckNewDir_ext, create(Command_DeckNewDirSchema, { path, dirName }), {
    onSuccess: () => {
      SessionPersistence.createServerDeckDir(path, dirName);
    },
  });
}
