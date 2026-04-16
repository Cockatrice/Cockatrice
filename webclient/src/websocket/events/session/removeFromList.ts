import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function removeFromList({ listName, userName }: Data.Event_RemoveFromList): void {
  switch (listName) {
    case 'buddy': {
      WebClient.instance.response.session.removeFromBuddyList(userName);
      break;
    }
    case 'ignore': {
      WebClient.instance.response.session.removeFromIgnoreList(userName);
      break;
    }
    default: {
      console.log(`Attempted to remove from unknown list: ${listName}`);
    }
  }
}
