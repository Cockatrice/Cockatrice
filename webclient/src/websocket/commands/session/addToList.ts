import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_AddToList_ext, Command_AddToListSchema } from '@app/generated';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Command_AddToList_ext, create(Command_AddToListSchema, { list, userName }), {
    onSuccess: () => {
      WebClient.instance.response.session.addToList(list, userName);
    },
  });
}
