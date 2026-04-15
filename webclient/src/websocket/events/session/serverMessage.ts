import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function serverMessage({ message }: Data.Event_ServerMessage): void {
  SessionPersistence.serverMessage(message);
}
