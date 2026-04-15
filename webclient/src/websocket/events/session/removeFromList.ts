import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function removeFromList({ listName, userName }: Data.Event_RemoveFromList): void {
  switch (listName) {
    case 'buddy': {
      SessionPersistence.removeFromBuddyList(userName);
      break;
    }
    case 'ignore': {
      SessionPersistence.removeFromIgnoreList(userName);
      break;
    }
    default: {
      console.log(`Attempted to remove from unknown list: ${listName}`);
    }
  }
}
