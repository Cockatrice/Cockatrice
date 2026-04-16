import type { Event_ListRooms } from '@app/generated';
import { CLIENT_OPTIONS } from '../../config';
import { joinRoom } from '../../commands/session';
import { WebClient } from '../../WebClient';

export function listRooms({ roomList }: Event_ListRooms): void {
  WebClient.instance.response.room.updateRooms(roomList);

  if (CLIENT_OPTIONS.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        joinRoom(roomId);
      }
    });
  }
}
