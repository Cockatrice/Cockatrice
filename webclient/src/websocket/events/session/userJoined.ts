import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function userJoined({ userInfo }: Data.Event_UserJoined): void {
  SessionPersistence.userJoined(userInfo);
}
