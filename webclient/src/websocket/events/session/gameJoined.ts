import { SessionPersistence } from '../../persistence';
import { GameJoinedData } from './interfaces';

export function gameJoined(gameJoined: GameJoinedData): void {
  SessionPersistence.gameJoined(gameJoined);
}
