import type { Data } from '@app/types';
import { SessionPersistence } from '../../persistence';

export function gameJoined(gameJoined: Data.Event_GameJoined): void {
  SessionPersistence.gameJoined(gameJoined);
}
