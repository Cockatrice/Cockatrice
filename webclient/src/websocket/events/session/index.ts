import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import {
  type RegistryEntry,
  type SessionEvent,
  makeEntry,
  Event_AddToList_ext,
  Event_ConnectionClosed_ext,
  Event_GameJoined_ext,
  Event_ListRooms_ext,
  Event_NotifyUser_ext,
  Event_RemoveFromList_ext,
  Event_ReplayAdded_ext,
  Event_ServerCompleteList_ext,
  Event_ServerIdentification_ext,
  Event_ServerMessage_ext,
  Event_ServerShutdown_ext,
  Event_UserJoined_ext,
  Event_UserLeft_ext,
  Event_UserMessage_ext,
} from '@app/generated';

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

type SessionRegistryEntry<V = unknown> = RegistryEntry<V, SessionEvent>;
export type SessionExtensionRegistry = SessionRegistryEntry[];

function makeSessionEntry<V>(
  ext: GenExtension<SessionEvent, V>,
  handler: (value: V) => void,
): SessionRegistryEntry {
  return makeEntry(ext, handler);
}

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
