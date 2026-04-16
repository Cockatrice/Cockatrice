import type { Data } from '@app/types';
import { WebClient } from '../../WebClient';

export function serverCompleteList({ userList, roomList }: Data.Event_ServerCompleteList): void {
  WebClient.instance.response.session.updateUsers(userList);
  WebClient.instance.response.room.updateRooms(roomList);
}
