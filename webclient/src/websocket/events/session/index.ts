import type { GenExtension } from '@bufbuild/protobuf/codegenv2';

import { Data } from '@app/types';

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

type SessionRegistryEntry<V = unknown> = Data.RegistryEntry<V, Data.SessionEvent>;
export type SessionExtensionRegistry = SessionRegistryEntry[];

function makeSessionEntry<V>(
  ext: GenExtension<Data.SessionEvent, V>,
  handler: (value: V) => void,
): SessionRegistryEntry {
  return Data.makeEntry(ext, handler);
}

export const SessionEvents: SessionExtensionRegistry = [
  makeSessionEntry(Data.Event_AddToList_ext, addToList),
  makeSessionEntry(Data.Event_ConnectionClosed_ext, connectionClosed),
  makeSessionEntry(Data.Event_GameJoined_ext, gameJoined),
  makeSessionEntry(Data.Event_ListRooms_ext, listRooms),
  makeSessionEntry(Data.Event_NotifyUser_ext, notifyUser),
  makeSessionEntry(Data.Event_RemoveFromList_ext, removeFromList),
  makeSessionEntry(Data.Event_ReplayAdded_ext, replayAdded),
  makeSessionEntry(Data.Event_ServerCompleteList_ext, serverCompleteList),
  makeSessionEntry(Data.Event_ServerIdentification_ext, serverIdentification),
  makeSessionEntry(Data.Event_ServerMessage_ext, serverMessage),
  makeSessionEntry(Data.Event_ServerShutdown_ext, serverShutdown),
  makeSessionEntry(Data.Event_UserJoined_ext, userJoined),
  makeSessionEntry(Data.Event_UserLeft_ext, userLeft),
  makeSessionEntry(Data.Event_UserMessage_ext, userMessage),
];
