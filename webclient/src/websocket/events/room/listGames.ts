import { RoomPersistence } from '../../persistence';
import { ListGamesData, RoomEvent } from './interfaces';

export function listGames({ gameList }: ListGamesData, { roomId }: RoomEvent): void {
  RoomPersistence.updateGames(roomId, gameList);
}
