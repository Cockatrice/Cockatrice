import { SessionPersistence } from '../../persistence';
import { UserJoinedData } from './interfaces';

export function userJoined({ userInfo }: UserJoinedData): void {
  SessionPersistence.userJoined(userInfo);
}
