import { create } from '@bufbuild/protobuf';
import { WebClient } from '../../WebClient';

import { Command_RemoveFromList_ext, Command_RemoveFromListSchema } from '@app/generated';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  WebClient.instance.protobuf.sendSessionCommand(
    Command_RemoveFromList_ext,
    create(Command_RemoveFromListSchema, { list, userName }),
    {
      onSuccess: () => {
        WebClient.instance.response.session.removeFromList(list, userName);
      },
    }
  );
}
