import { CLIENT_OPTIONS } from '../../config';
import { joinRoom } from '../../commands/session';
import { RoomPersistence } from '../../persistence';
import { ListRoomsData } from './interfaces';

export function listRooms({ roomList }: ListRoomsData): void {
  RoomPersistence.updateRooms(roomList);

  if (CLIENT_OPTIONS.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        joinRoom(roomId);
      }
    });
  }
}
