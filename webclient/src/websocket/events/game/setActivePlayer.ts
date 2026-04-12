import { GameEventMeta, SetActivePlayerData } from 'types';
import { GamePersistence } from '../../persistence';

export function setActivePlayer(data: SetActivePlayerData, meta: GameEventMeta): void {
  GamePersistence.activePlayerSet(meta.gameId, data.activePlayerId);
}
