import { GameEventMeta, PlayerProperties } from 'types';
import { GamePersistence } from '../../persistence';

export function playerPropertiesChanged(data: { playerProperties: PlayerProperties }, meta: GameEventMeta): void {
  GamePersistence.playerPropertiesChanged(meta.gameId, meta.playerId, data.playerProperties);
}
