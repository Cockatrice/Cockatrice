import type { Data } from '@app/types';
import { RoomPersistence, SessionPersistence } from '../../persistence';

export function serverCompleteList({ userList, roomList }: Data.Event_ServerCompleteList): void {
  SessionPersistence.updateUsers(userList);
  RoomPersistence.updateRooms(roomList);
}
