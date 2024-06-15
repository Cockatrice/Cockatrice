import { RoomPersistence } from '../../persistence';
import { RemoveMessagesData, RoomEvent } from './interfaces';

export function removeMessages({ name }: RemoveMessagesData, { roomEvent }: RoomEvent) {
  const { roomId } = roomEvent;
  RoomPersistence.removeMessages(roomId, name);
}
