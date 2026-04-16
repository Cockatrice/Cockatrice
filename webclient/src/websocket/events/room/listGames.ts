import type { Event_ListGames, RoomEvent } from '@app/generated';
import { WebClient } from '../../WebClient';

export function listGames({ gameList }: Event_ListGames, { roomId }: RoomEvent): void {
  WebClient.instance.response.room.updateGames(roomId, gameList);
}
