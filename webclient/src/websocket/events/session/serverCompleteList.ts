import { RoomPersistence, SessionPersistence } from '../../persistence';
import { ServerCompleteListData } from './interfaces';

export function serverCompleteList({ userList, roomList }: ServerCompleteListData): void {
  SessionPersistence.updateUsers(userList);
  RoomPersistence.updateRooms(roomList);
}
