import { ProtobufEvents } from '../../services/ProtobufService';
import { addToList } from './addToList';
import { connectionClosed } from './connectionClosed';
import { listRooms } from './listRooms';
import { notifyUser } from './notifyUser';
import { playerPropertiesChanged } from '../common/playerPropertiesChanged';
import { removeFromList } from './removeFromList';
import { serverIdentification } from './serverIdentification';
import { serverMessage } from './serverMessage';
import { serverShutdown } from './serverShutdown';
import { userJoined } from './userJoined';
import { userLeft } from './userLeft';
import { userMessage } from './userMessage';
import { gameJoined } from './gameJoined';

export const SessionEvents: ProtobufEvents = {
  '.Event_AddToList.ext': addToList,
  '.Event_ConnectionClosed.ext': connectionClosed,
  '.Event_GameJoined.ext': gameJoined,
  '.Event_ListRooms.ext': listRooms,
  '.Event_NotifyUser.ext': notifyUser,
  '.Event_RemoveFromList.ext': removeFromList,
  '.Event_ReplayAdded.ext': () => console.log('Event_ReplayAdded'),
  '.Event_ServerCompleteList.ext': () => console.log('Event_ServerCompleteList'),
  '.Event_ServerIdentification.ext': serverIdentification,
  '.Event_ServerMessage.ext': serverMessage,
  '.Event_ServerShutdown.ext': serverShutdown,
  '.Event_UserJoined.ext': userJoined,
  '.Event_UserLeft.ext': userLeft,
  '.Event_UserMessage.ext': userMessage,
}
