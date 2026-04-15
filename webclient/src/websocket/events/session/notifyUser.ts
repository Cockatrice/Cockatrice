import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function notifyUser(payload: Data.Event_NotifyUser): void {
  SessionPersistence.notifyUser(payload);
}
