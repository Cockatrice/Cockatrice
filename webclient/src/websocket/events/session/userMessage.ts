import { SessionPersistence } from '../../persistence';
import { UserMessageData } from './interfaces';



export function userMessage(payload: UserMessageData): void {
  SessionPersistence.userMessage(payload);
}
