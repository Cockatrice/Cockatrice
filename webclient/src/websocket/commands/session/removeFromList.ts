import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function removeFromBuddyList(userName: string): void {
  removeFromList('buddy', userName);
}

export function removeFromIgnoreList(userName: string): void {
  removeFromList('ignore', userName);
}

export function removeFromList(list: string, userName: string): void {
  BackendService.sendSessionCommand('Command_RemoveFromList', { list, userName }, {
    onSuccess: () => {
      SessionPersistence.removeFromList(list, userName);
    },
  });
}
