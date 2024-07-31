import { GamePersistence } from '../../persistence';
import { PlayerGamePropertiesData } from '../session/interfaces';

export function joinGame(playerGamePropertiesData: PlayerGamePropertiesData): void {
  GamePersistence.joinGame(playerGamePropertiesData);
}
