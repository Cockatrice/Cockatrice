import { ChangeZonePropertiesData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function changeZoneProperties(data: ChangeZonePropertiesData, meta: GameEventMeta): void {
  GamePersistence.zonePropertiesChanged(meta.gameId, meta.playerId, data);
}
