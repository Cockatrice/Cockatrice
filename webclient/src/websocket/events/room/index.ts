import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import {
  type RegistryEntry,
  type RoomEvent,
  makeEntry,
  Event_JoinRoom_ext,
  Event_LeaveRoom_ext,
  Event_ListGames_ext,
  Event_RemoveMessages_ext,
  Event_RoomSay_ext,
} from '@app/generated';

import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { roomSay } from './roomSay';
import { removeMessages } from './removeMessages';

type RoomRegistryEntry<V = unknown> = RegistryEntry<V, RoomEvent, RoomEvent>;
export type RoomExtensionRegistry = RoomRegistryEntry[];

function makeRoomEntry<V>(
  ext: GenExtension<RoomEvent, V>,
  handler: (value: V, roomEvent: RoomEvent) => void,
): RoomRegistryEntry {
  return makeEntry(ext, handler);
}

export const RoomEvents: RoomExtensionRegistry = [
  makeRoomEntry(Event_JoinRoom_ext, joinRoom),
  makeRoomEntry(Event_LeaveRoom_ext, leaveRoom),
  makeRoomEntry(Event_ListGames_ext, listGames),
  makeRoomEntry(Event_RemoveMessages_ext, removeMessages),
  makeRoomEntry(Event_RoomSay_ext, roomSay),
];
