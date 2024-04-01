import { SessionPersistence } from '../../persistence';
import { ServerMessageData } from './interfaces';

export function serverMessage({ message }: ServerMessageData) {
  SessionPersistence.serverMessage(message);
}
