import type { Event_GameSay } from 'generated/proto/event_game_say_pb';
import type { GameEventMeta } from 'types';
import { GamePersistence } from '../../persistence';

export function gameSay(data: Event_GameSay, meta: GameEventMeta): void {
  GamePersistence.gameSay(meta.gameId, meta.playerId, data.message);
}
