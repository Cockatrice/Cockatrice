import { RoomPersistence } from '../../persistence';
import { JoinRoomData, RoomEvent } from './interfaces';

export function joinRoom({ userInfo }: JoinRoomData, { roomId }: RoomEvent): void {
  RoomPersistence.userJoined(roomId, userInfo);
}
