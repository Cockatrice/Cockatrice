import { GameEventMeta, ReverseTurnData } from 'types';
import { GamePersistence } from '../../persistence';

export function reverseTurn(data: ReverseTurnData, meta: GameEventMeta): void {
  GamePersistence.turnReversed(meta.gameId, data.reversed);
}
