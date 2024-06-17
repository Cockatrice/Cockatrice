import webClient from '../../WebClient';
import { joinRoom } from '../../commands/session';
import { RoomPersistence } from '../../persistence';
import { ListRoomsData } from './interfaces';

export function listRooms({ roomList }: ListRoomsData): void {
  RoomPersistence.updateRooms(roomList);

  if (webClient.clientOptions.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        joinRoom(roomId);
      }
    });
  }
}
