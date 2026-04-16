import type { Data } from '@app/types';
import { CLIENT_OPTIONS } from '../../config';
import { joinRoom } from '../../commands/session';
import { WebClient } from '../../WebClient';

export function listRooms({ roomList }: Data.Event_ListRooms): void {
  WebClient.instance.response.room.updateRooms(roomList);

  if (CLIENT_OPTIONS.autojoinrooms) {
    roomList.forEach(({ autoJoin, roomId }) => {
      if (autoJoin) {
        joinRoom(roomId);
      }
    });
  }
}
