import { SessionPersistence } from '../../persistence';
import { NotifyUserData } from './interfaces';


export function notifyUser(payload: NotifyUserData): void {
  SessionPersistence.notifyUser(payload);
}
