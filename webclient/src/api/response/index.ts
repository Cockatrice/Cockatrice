import type { IWebClientResponse } from '@app/websocket';

import { SessionResponseImpl } from './SessionResponseImpl';
import { RoomResponseImpl } from './RoomResponseImpl';
import { GameResponseImpl } from './GameResponseImpl';
import { AdminResponseImpl } from './AdminResponseImpl';
import { ModeratorResponseImpl } from './ModeratorResponseImpl';

export { SessionResponseImpl, RoomResponseImpl, GameResponseImpl, AdminResponseImpl, ModeratorResponseImpl };

export function createWebClientResponse(): IWebClientResponse {
  return {
    session: new SessionResponseImpl(),
    room: new RoomResponseImpl(),
    game: new GameResponseImpl(),
    admin: new AdminResponseImpl(),
    moderator: new ModeratorResponseImpl(),
  };
}
