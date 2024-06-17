import { RoomPersistence } from '../../persistence';
import { RemoveMessagesData, RoomEvent } from './interfaces';

export function removeMessages({ name, amount }: RemoveMessagesData, { roomEvent: { roomId } }: RoomEvent) {
  RoomPersistence.removeMessages(roomId, name, amount);
}
