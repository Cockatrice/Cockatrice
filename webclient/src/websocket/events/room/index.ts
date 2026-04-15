import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { Data } from '@app/types';

import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { roomSay } from './roomSay';
import { removeMessages } from './removeMessages';

type RoomRegistryEntry<V = unknown> = Data.RegistryEntry<V, Data.RoomEvent, Data.RoomEvent>;
export type RoomExtensionRegistry = RoomRegistryEntry[];

function makeRoomEntry<V>(
  ext: GenExtension<Data.RoomEvent, V>,
  handler: (value: V, roomEvent: Data.RoomEvent) => void,
): RoomRegistryEntry {
  return Data.makeEntry(ext, handler);
}

export const RoomEvents: RoomExtensionRegistry = [
  makeRoomEntry(Data.Event_JoinRoom_ext, joinRoom),
  makeRoomEntry(Data.Event_LeaveRoom_ext, leaveRoom),
  makeRoomEntry(Data.Event_ListGames_ext, listGames),
  makeRoomEntry(Data.Event_RemoveMessages_ext, removeMessages),
  makeRoomEntry(Data.Event_RoomSay_ext, roomSay),
];
