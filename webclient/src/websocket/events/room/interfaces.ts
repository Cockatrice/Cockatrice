import type { Event_JoinRoom } from 'generated/proto/event_join_room_pb';
import type { Event_LeaveRoom } from 'generated/proto/event_leave_room_pb';
import type { Event_ListGames } from 'generated/proto/event_list_games_pb';
import type { Event_RemoveMessages } from 'generated/proto/event_remove_messages_pb';
import type { Event_RoomSay } from 'generated/proto/event_room_say_pb';
import type { RoomEvent as GeneratedRoomEvent } from 'generated/proto/room_event_pb';

export type JoinRoomData = Event_JoinRoom;
export type LeaveRoomData = Event_LeaveRoom;
export type ListGamesData = Event_ListGames;
export type RemoveMessagesData = Event_RemoveMessages;
export type RoomSayData = Event_RoomSay;
export type RoomEvent = GeneratedRoomEvent;
