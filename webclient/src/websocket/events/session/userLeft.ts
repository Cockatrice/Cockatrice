import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function userLeft({ name }: Data.Event_UserLeft): void {
  SessionPersistence.userLeft(name);
}
