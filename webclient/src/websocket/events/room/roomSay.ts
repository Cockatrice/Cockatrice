import type { Data } from '@app/types';
import { Enriched } from '@app/types';
import { WebClient } from '../../WebClient';


export function roomSay(data: Data.Event_RoomSay, { roomId }: Data.RoomEvent): void {
  const message: Enriched.Message = { ...data, timeReceived: Date.now() };
  WebClient.instance.response.room.addMessage(roomId, message);
}
