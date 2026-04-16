import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Data } from '@app/types';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(Data.Command_AddToList_ext, create(Data.Command_AddToListSchema, { list, userName }), {
    onSuccess: () => {
      WebClient.instance.response.session.addToList(list, userName);
    },
  });
}
