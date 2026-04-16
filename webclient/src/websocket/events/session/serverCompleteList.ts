import type { Event_ServerCompleteList } from '@app/generated';
import { WebClient } from '../../WebClient';

export function serverCompleteList({ userList, roomList }: Event_ServerCompleteList): void {
  WebClient.instance.response.session.updateUsers(userList);
  WebClient.instance.response.room.updateRooms(roomList);
}
