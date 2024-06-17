import { PlayerGamePropertiesData } from './interfaces';
import { SessionPersistence } from '../../persistence';

export function playerPropertiesChanges(payload: PlayerGamePropertiesData): void {
  SessionPersistence.playerPropertiesChanged(payload);
}
