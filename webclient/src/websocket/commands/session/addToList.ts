import { create } from '@bufbuild/protobuf';
import { BackendService } from '../../services/BackendService';
import { Command_AddToList_ext, Command_AddToListSchema } from 'generated/proto/session_commands_pb';
import { SessionPersistence } from '../../persistence';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  BackendService.sendSessionCommand(Command_AddToList_ext, create(Command_AddToListSchema, { list, userName }), {
    onSuccess: () => {
      SessionPersistence.addToList(list, userName);
    },
  });
}
