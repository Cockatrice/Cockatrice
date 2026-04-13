import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_RemoveFromList_ext, Command_RemoveFromListSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  BackendService.sendSessionCommand(Command_RemoveFromList_ext, create(Command_RemoveFromListSchema, { list, userName }), {
    onSuccess: () => {
      SessionPersistence.removeFromList(list, userName);
    },
  });
}
