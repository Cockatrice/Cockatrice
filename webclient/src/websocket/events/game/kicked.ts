import { Enriched } from '@app/types';
import { GamePersistence } from '../../persistence';

export function kicked(_data: {}, meta: Enriched.GameEventMeta): void {
  GamePersistence.kicked(meta.gameId);
}
