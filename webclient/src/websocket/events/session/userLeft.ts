import { SessionPersistence } from '../../persistence';
import { UserLeftData } from './interfaces';

export function userLeft({ name }: UserLeftData) {
  SessionPersistence.userLeft(name);
}
