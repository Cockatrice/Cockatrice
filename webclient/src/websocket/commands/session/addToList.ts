import { BackendService } from '../../services/BackendService';
import { SessionPersistence } from '../../persistence';

export function addToBuddyList(userName: string): void {
  addToList('buddy', userName);
}

export function addToIgnoreList(userName: string): void {
  addToList('ignore', userName);
}

export function addToList(list: string, userName: string): void {
  BackendService.sendSessionCommand('Command_AddToList', { list, userName }, {
    onSuccess: () => {
      SessionPersistence.addToList(list, userName);
    },
  });
}
