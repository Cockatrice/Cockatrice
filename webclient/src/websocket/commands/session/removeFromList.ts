import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_RemoveFromList_ext, create(Data.Command_RemoveFromListSchema, { list, userName }), {
    onSuccess: () => {
      SessionPersistence.removeFromList(list, userName);
    },
  });
}
