import type { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';

export function listGames({ gameList }: Data.Event_ListGames, { roomId }: Data.RoomEvent): void {
  RoomPersistence.updateGames(roomId, gameList);
}
