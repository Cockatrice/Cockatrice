import { RoomExtensionRegistry, makeRoomEntry } from '../../services/ProtobufService';

import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { roomSay } from './roomSay';
import { removeMessages } from './removeMessages';

import { Event_JoinRoom_ext } from 'generated/proto/event_join_room_pb';
import { Event_LeaveRoom_ext } from 'generated/proto/event_leave_room_pb';
import { Event_ListGames_ext } from 'generated/proto/event_list_games_pb';
import { Event_RemoveMessages_ext } from 'generated/proto/event_remove_messages_pb';
import { Event_RoomSay_ext } from 'generated/proto/event_room_say_pb';

export const RoomEvents: RoomExtensionRegistry = [
  makeRoomEntry(Event_JoinRoom_ext, joinRoom),
  makeRoomEntry(Event_LeaveRoom_ext, leaveRoom),
  makeRoomEntry(Event_ListGames_ext, listGames),
  makeRoomEntry(Event_RemoveMessages_ext, removeMessages),
  makeRoomEntry(Event_RoomSay_ext, roomSay),
];

