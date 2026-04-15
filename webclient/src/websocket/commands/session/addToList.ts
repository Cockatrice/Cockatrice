import { create } from '@bufbuild/protobuf';
import webClient from '../../WebClient';

import { SessionPersistence } from '../../persistence';
import { Data } from '@app/types';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  webClient.protobuf.sendSessionCommand(Data.Command_AddToList_ext, create(Data.Command_AddToListSchema, { list, userName }), {
    onSuccess: () => {
      SessionPersistence.addToList(list, userName);
    },
  });
}
