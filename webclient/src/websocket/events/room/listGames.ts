import { RoomPersistence } from '../../persistence';
import { ListGamesData, RoomEvent } from './interfaces';

export function listGames({ gameList }: ListGamesData, { roomEvent: { roomId } }: RoomEvent): void {
  RoomPersistence.updateGames(roomId, gameList);
}
