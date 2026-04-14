import type { Event_CreateToken } from 'generated/proto/event_create_token_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function createToken(data: Event_CreateToken, meta: GameEventMeta): void {
  GamePersistence.tokenCreated(meta.gameId, meta.playerId, data);
}
