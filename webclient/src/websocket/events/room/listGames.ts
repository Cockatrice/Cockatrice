import { RoomPersistence } from '../../persistence';
import { ListGamesData, RoomEvent } from './interfaces';

export function listGames({ gameList }: ListGamesData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.updateGames(roomId, gameList);
}
