import { SessionPersistence } from '../../persistence';
import { RemoveFromListData } from './interfaces';

export function removeFromList({ listName, userName }: RemoveFromListData): void {
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
