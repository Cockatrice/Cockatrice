import type { Data } from '@app/types';
import { RoomPersistence } from '../../persistence';

export function removeMessages({ name, amount }: Data.Event_RemoveMessages, { roomId }: Data.RoomEvent): void {
  RoomPersistence.removeMessages(roomId, name, amount);
}
