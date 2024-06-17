import { SessionPersistence } from '../../persistence';
import { GameJoinedData } from './interfaces';

export function gameJoined(gameJoined: GameJoinedData) {
  SessionPersistence.gameJoined(gameJoined);
}
