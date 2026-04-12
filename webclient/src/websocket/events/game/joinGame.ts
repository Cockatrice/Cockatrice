import { GamePersistence } from '../../persistence';
import { GameEventMeta, PlayerProperties } from 'types';

export function joinGame(data: { playerProperties: PlayerProperties }, meta: GameEventMeta): void {
  GamePersistence.playerJoined(meta.gameId, data.playerProperties);
}
