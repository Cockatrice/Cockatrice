import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function addToList({ listName, userInfo }: Data.Event_AddToList): void {
  switch (listName) {
    case 'buddy': {
      WebClient.instance.response.session.addToBuddyList(userInfo);
      break;
    }
    case 'ignore': {
      WebClient.instance.response.session.addToIgnoreList(userInfo);
      break;
    }
    default: {
      console.log(`Attempted to add to unknown list: ${listName}`);
    }
  }
}
