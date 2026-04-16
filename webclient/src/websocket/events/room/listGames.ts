import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function listGames({ gameList }: Data.Event_ListGames, { roomId }: Data.RoomEvent): void {
  WebClient.instance.response.room.updateGames(roomId, gameList);
}
