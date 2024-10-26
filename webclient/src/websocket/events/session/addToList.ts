import { SessionPersistence } from '../../persistence';
import { AddToListData } from './interfaces';

export function addToList({ listName, userInfo }: AddToListData): void {
  switch (listName) {
    case 'buddy': {
      SessionPersistence.addToBuddyList(userInfo);
      break;
    }
    case 'ignore': {
      SessionPersistence.addToIgnoreList(userInfo);
      break;
    }
    default: {
      console.log(`Attempted to add to unknown list: ${listName}`);
    }
  }
}
