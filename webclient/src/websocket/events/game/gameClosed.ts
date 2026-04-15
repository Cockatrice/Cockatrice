import { Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function gameClosed(_data: {}, meta: Enriched.GameEventMeta): void {
  GamePersistence.gameClosed(meta.gameId);
}
