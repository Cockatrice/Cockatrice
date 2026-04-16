import type { IWebClientResponse } from '@app/websocket';

import { SessionResponseImpl } from './SessionResponseImpl';
import { RoomResponseImpl } from './RoomResponseImpl';
import { GameResponseImpl } from './GameResponseImpl';
import { AdminResponseImpl } from './AdminResponseImpl';
import { ModeratorResponseImpl } from './ModeratorResponseImpl';

export { SessionResponseImpl } from './SessionResponseImpl';
export { RoomResponseImpl } from './RoomResponseImpl';
export { GameResponseImpl } from './GameResponseImpl';
export { AdminResponseImpl } from './AdminResponseImpl';
export { ModeratorResponseImpl } from './ModeratorResponseImpl';

export function createWebClientResponse(): IWebClientResponse {
  return {
    session: new SessionResponseImpl(),
    room: new RoomResponseImpl(),
    game: new GameResponseImpl(),
    admin: new AdminResponseImpl(),
    moderator: new ModeratorResponseImpl(),
  };
}
