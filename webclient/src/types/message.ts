import type { Event_RoomSay } from 'generated/proto/event_room_say_pb';

export type Message = Event_RoomSay & {
  timeReceived: number;
};
