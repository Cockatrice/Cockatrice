import { SessionPersistence } from '../../persistence';
import { ServerShutdownData } from './interfaces';


export function serverShutdown(payload: ServerShutdownData): void {
  SessionPersistence.serverShutdown(payload);
}
