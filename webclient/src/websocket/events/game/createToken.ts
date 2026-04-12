import { CreateTokenData, GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createToken(data: CreateTokenData, meta: GameEventMeta): void {
  GamePersistence.tokenCreated(meta.gameId, meta.playerId, data);
}
