import { SessionExtensionRegistry, makeSessionEntry } from '../../services/ProtobufService';
import { addToList } from './addToList';
import { connectionClosed } from './connectionClosed';
import { listRooms } from './listRooms';
import { notifyUser } from './notifyUser';
import { removeFromList } from './removeFromList';
import { replayAdded } from './replayAdded';
import { serverCompleteList } from './serverCompleteList';
import { serverIdentification } from './serverIdentification';
import { serverMessage } from './serverMessage';
import { serverShutdown } from './serverShutdown';
import { userJoined } from './userJoined';
import { userLeft } from './userLeft';
import { userMessage } from './userMessage';
import { gameJoined } from './gameJoined';

import { Event_AddToList_ext } from 'generated/proto/event_add_to_list_pb';
import { Event_ConnectionClosed_ext } from 'generated/proto/event_connection_closed_pb';
import { Event_GameJoined_ext } from 'generated/proto/event_game_joined_pb';
import { Event_ListRooms_ext } from 'generated/proto/event_list_rooms_pb';
import { Event_NotifyUser_ext } from 'generated/proto/event_notify_user_pb';
import { Event_RemoveFromList_ext } from 'generated/proto/event_remove_from_list_pb';
import { Event_ReplayAdded_ext } from 'generated/proto/event_replay_added_pb';
import { Event_ServerCompleteList_ext } from 'generated/proto/event_server_complete_list_pb';
import { Event_ServerIdentification_ext } from 'generated/proto/event_server_identification_pb';
import { Event_ServerMessage_ext } from 'generated/proto/event_server_message_pb';
import { Event_ServerShutdown_ext } from 'generated/proto/event_server_shutdown_pb';
import { Event_UserJoined_ext } from 'generated/proto/event_user_joined_pb';
import { Event_UserLeft_ext } from 'generated/proto/event_user_left_pb';
import { Event_UserMessage_ext } from 'generated/proto/event_user_message_pb';

export const SessionEvents: SessionExtensionRegistry = [
  makeSessionEntry(Event_AddToList_ext, addToList),
  makeSessionEntry(Event_ConnectionClosed_ext, connectionClosed),
  makeSessionEntry(Event_GameJoined_ext, gameJoined),
  makeSessionEntry(Event_ListRooms_ext, listRooms),
  makeSessionEntry(Event_NotifyUser_ext, notifyUser),
  makeSessionEntry(Event_RemoveFromList_ext, removeFromList),
  makeSessionEntry(Event_ReplayAdded_ext, replayAdded),
  makeSessionEntry(Event_ServerCompleteList_ext, serverCompleteList),
  makeSessionEntry(Event_ServerIdentification_ext, serverIdentification),
  makeSessionEntry(Event_ServerMessage_ext, serverMessage),
  makeSessionEntry(Event_ServerShutdown_ext, serverShutdown),
  makeSessionEntry(Event_UserJoined_ext, userJoined),
  makeSessionEntry(Event_UserLeft_ext, userLeft),
  makeSessionEntry(Event_UserMessage_ext, userMessage),
];

