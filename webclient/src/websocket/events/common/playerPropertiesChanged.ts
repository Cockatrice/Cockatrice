import { PlayerGamePropertiesData } from '../session/interfaces';
import { SessionPersistence } from '../../persistence';

export function playerPropertiesChanged(payload: PlayerGamePropertiesData): void {
  SessionPersistence.playerPropertiesChanged(payload);
}
