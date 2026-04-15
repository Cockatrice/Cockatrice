import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function serverShutdown(payload: Data.Event_ServerShutdown): void {
  SessionPersistence.serverShutdown(payload);
}
