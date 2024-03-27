import { ProtobufEvents } from '../../services/ProtobufService';

import { joinRoom } from './joinRoom';
import { leaveRoom } from './leaveRoom';
import { listGames } from './listGames';
import { roomSay } from './roomSay';

export const RoomEvents: ProtobufEvents = {
  '.Event_JoinRoom.ext': joinRoom,
  '.Event_LeaveRoom.ext': leaveRoom,
  '.Event_ListGames.ext': listGames,
  '.Event_RoomSay.ext': roomSay,
};
