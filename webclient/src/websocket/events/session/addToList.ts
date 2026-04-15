import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function addToList({ listName, userInfo }: Data.Event_AddToList): void {
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
