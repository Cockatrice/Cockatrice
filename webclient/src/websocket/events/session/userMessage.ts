import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function userMessage(payload: Data.Event_UserMessage): void {
  SessionPersistence.userMessage(payload);
}
