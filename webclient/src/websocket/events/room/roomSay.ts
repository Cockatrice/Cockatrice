import type { Data } from '@app/types';
import { Enriched } from '@app/types';

import { RoomPersistence } from '../../persistence';

export function roomSay(data: Data.Event_RoomSay, { roomId }: Data.RoomEvent): void {
  const message: Enriched.Message = { ...data, timeReceived: Date.now() };
  RoomPersistence.addMessage(roomId, message);
}
